# -*- coding: utf-8 -*-
"""
@File  : rotation.py
@Author: Yulong He
@Date  : 2021/11/11 11:18
@Desc  : 
"""
import os
import numpy as np
import torch
from torch.nn import functional as F
from py_anim.rotation import mat_l1_loss as mat_loss
from py_anim.rotation import vicon_rot_id_map, compute_rotation_matrix_from_ortho6d_torch
from py_anim.animation_ext import Animation
from iknet.config import ROT_JNUM, ALL_JNUM

std_anim = Animation.from_bvh_filename(os.path.join(os.path.dirname(__file__), '../assets/ViconMaleZYX.bvh'))  # 标准 VICON 骨架

# device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
std_offsets_torch = torch.from_numpy(std_anim.offsets.astype(np.float32))


def _relative_fk(ortho6d):
    rotation_matrix = compute_rotation_matrix_from_ortho6d_torch(ortho6d)
    BT = rotation_matrix.shape[0]
    rotation_matrix33 = torch.zeros((BT, ALL_JNUM, 3, 3)).to(rotation_matrix.device)
    rotation_matrix33[:, vicon_rot_id_map] = rotation_matrix

    xyz = torch.zeros(BT, ALL_JNUM, 3).to(rotation_matrix.device)
    for i, pi in enumerate(std_anim.parents):
        if pi == -1:
            continue
        # https://www.yongfengli.tk/2018/04/13/inplace-operation-in-pytorch.html#:~:text=RuntimeError%3A%20one%20of%20the%20variables%20needed%20for%20gradient,some%20inplace%20operations%20and%20change%20the%20%20value.
        rotation_matrix33[:, i, :, :] = torch.matmul(rotation_matrix33[:, pi, :, :].clone(),
                                                     rotation_matrix33[..., i, :, :].clone())
        xyz[:, i, :] = torch.matmul(rotation_matrix33[..., pi, :, :],
                                    std_offsets_torch[i].to(rotation_matrix.device)) + xyz[..., pi, :]

    return xyz


def fk_l1_loss(batch, y_hat, model, stage):
    xyz_gt, _ = batch
    B = y_hat.shape[0]
    q6_hat = y_hat.view(B, ROT_JNUM, 6)

    xyz_hat = _relative_fk(q6_hat)
    result = F.l1_loss(xyz_gt, xyz_hat)
    model.log(f'{stage}/fk_l1_loss', result)
    return result


def mat_l1_loss(batch, y_hat, model, stage):
    _, y = batch
    B = y.shape[0]
    y_hat = y_hat.view(B, ROT_JNUM, 6)
    y_gt = y[:, vicon_rot_id_map]
    result = mat_loss(y_gt.view(-1, 6), y_hat.view(-1, 6))
    model.log(f'{stage}/mat_l1_loss', result)
    return result
