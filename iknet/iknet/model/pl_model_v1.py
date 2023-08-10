# -*- coding: utf-8 -*-
"""
@File  : pl_model_v1.py
@Author: Yulong He
@Date  : 2021/11/11 10:59
@Desc  : 
"""
from torch.optim import Adam, lr_scheduler, AdamW
from torch import nn
import torch.nn.functional as F
import pytorch_lightning as pl
from iknet.config import DEFULAT_LR, DEFAULT_USE_LR_STEP, DEFAULT_LR_STEP_SIZE, DEFAULT_LR_GAMMA, \
    DEFAULT_WEIGHT_DECAY
import os
from pytorch_lightning.callbacks import ModelCheckpoint, LearningRateMonitor
from iknet.pl_data import DataModule
from iknet.constraint.rotation import fk_l1_loss, mat_l1_loss
from iknet.config import INPUT_ID, INPUT_ID2, ROT_JNUM, INPUT_XYZ_NAMES, INPUT_XYZ_NAMES2


class NaiveDNN(pl.LightningModule):
    def __init__(self, in_dim=len(INPUT_XYZ_NAMES) * 3, latent_width=512, out_dim=ROT_JNUM * 6,
                 lr=DEFULAT_LR,
                 use_lr_step=DEFAULT_USE_LR_STEP,
                 lr_step_size=DEFAULT_LR_STEP_SIZE,
                 lr_gamma=DEFAULT_LR_GAMMA,
                 weight_decay=DEFAULT_WEIGHT_DECAY,
                 **kwargs):
        super().__init__()
        try:
            self.save_hyperparameters()
        except Exception as e:
            print(f'Saving hyper-parameters error!!!\n{e}')
        # ====================================================
        self.lr = lr
        self.use_lr_step = bool(use_lr_step)
        if self.use_lr_step:
            self.lr_step_size = lr_step_size
            self.lr_gamma = lr_gamma
        self.weight_decay = weight_decay
        # ====================================================
        self.dense1 = nn.Linear(in_dim, latent_width)
        self.dense2 = nn.Linear(latent_width, latent_width)
        self.dense3 = nn.Linear(latent_width, latent_width)
        self.dense4 = nn.Linear(latent_width, latent_width)
        self.dense5 = nn.Linear(latent_width, out_dim)
        # ====================================================
        self.loss_functions = [
            # (1e-1, lcl_q6_smoothing_loss),
            # (1e-1, lcl_q6_mse_loss),
            (1, mat_l1_loss),
            (1, fk_l1_loss),
            # (1, root_mse_loss),
            # (contact_label_multiplier, contact_bce_loss),
            # (contact_label_multiplier, penalty_contact_velocity_and_penetration),
            # (contact_position_multiplier, contact_xyz_loss)
        ]

    @staticmethod
    def _q_normalized(q):
        old_shape = q.shape
        q = q.reshape(-1, 3)
        return F.normalize(q, p=2, dim=-1).view(old_shape)

    def forward(self, x):
        x = F.leaky_relu(self.dense1(x))
        x = F.leaky_relu(self.dense2(x)) + x
        x = F.leaky_relu(self.dense3(x)) + x
        x = F.leaky_relu(self.dense4(x)) + x
        x = self.dense5(x)
        y = self._q_normalized(x)
        return y

    def _cal_loss(self, batch, y_hat, stage):
        assert len(self.loss_functions) > 0

        for i, (k, f) in enumerate(self.loss_functions):
            loss = f(batch, y_hat, self, stage)
            if i == 0:
                all_loss = k * loss
            else:
                all_loss = all_loss + k * loss

        self.log(f'{stage}/normal_loss', all_loss)
        return all_loss

    def training_step(self, batch, batch_idx):
        x, y = batch
        x_in = x[:, INPUT_ID]
        y_hat = self.forward(x_in.view(x.size(0), -1))
        return self._cal_loss(batch, y_hat, 'train')

    def validation_step(self, batch, batch_idx):
        x, y = batch
        x_in = x[:, INPUT_ID]
        y_hat = self.forward(x_in.view(x.size(0), -1))
        self._cal_loss(batch, y_hat, 'val')

    def configure_optimizers(self):
        """
        https://pytorch-lightning.readthedocs.io/en/latest/common/optimizers.html?highlight=lr_scheduler#learning-rate-scheduling
        :return:
        """
        optimizer = Adam(self.parameters(), lr=self.lr, weight_decay=self.weight_decay)
        # optimizer = AdamW(self.parameters(), lr=self.lr, weight_decay=self.weight_decay)
        if self.use_lr_step:
            scheduler = lr_scheduler.StepLR(optimizer, step_size=self.lr_step_size, gamma=self.lr_gamma)
            return [optimizer], [scheduler]
        else:
            return optimizer

    @staticmethod
    def add_model_specific_args(parent_parser):
        parser = parent_parser.add_argument_group('NaiveDNN')
        parser.add_argument('--restored_model_path', type=str, default='')
        parser.add_argument('--lr', type=float, default=1e-3)
        parser.add_argument('--use_lr_step', type=int, default=DEFAULT_USE_LR_STEP)
        parser.add_argument('--lr_step_size', type=float, default=DEFAULT_LR_STEP_SIZE)
        parser.add_argument('--lr_gamma', type=float, default=DEFAULT_LR_GAMMA)
        parser.add_argument('--weight_decay', type=float, default=DEFAULT_WEIGHT_DECAY)


class NaiveDNN2(pl.LightningModule):
    def __init__(self, in_dim=len(INPUT_XYZ_NAMES2) * 3, latent_width=512, out_dim=ROT_JNUM * 6,
                 lr=DEFULAT_LR,
                 use_lr_step=DEFAULT_USE_LR_STEP,
                 lr_step_size=DEFAULT_LR_STEP_SIZE,
                 lr_gamma=DEFAULT_LR_GAMMA,
                 weight_decay=DEFAULT_WEIGHT_DECAY,
                 **kwargs):
        super().__init__()
        try:
            self.save_hyperparameters()
        except Exception as e:
            print(f'Saving hyper-parameters error!!!\n{e}')
        # ====================================================
        self.lr = lr
        self.use_lr_step = bool(use_lr_step)
        if self.use_lr_step:
            self.lr_step_size = lr_step_size
            self.lr_gamma = lr_gamma
        self.weight_decay = weight_decay
        # ====================================================
        self.dense1 = nn.Linear(in_dim, latent_width)
        self.dense2 = nn.Linear(latent_width, latent_width)
        self.dense3 = nn.Linear(latent_width, latent_width)
        self.dense4 = nn.Linear(latent_width, latent_width)
        self.dense5 = nn.Linear(latent_width, out_dim)
        # ====================================================
        self.loss_functions = [
            # (1e-1, lcl_q6_smoothing_loss),
            # (1e-1, lcl_q6_mse_loss),
            (1, mat_l1_loss),
            (1, fk_l1_loss),
            # (1, root_mse_loss),
            # (contact_label_multiplier, contact_bce_loss),
            # (contact_label_multiplier, penalty_contact_velocity_and_penetration),
            # (contact_position_multiplier, contact_xyz_loss)
        ]

    @staticmethod
    def _q_normalized(q):
        old_shape = q.shape
        q = q.reshape(-1, 3)
        return F.normalize(q, p=2, dim=-1).view(old_shape)

    def forward(self, x):
        x = F.leaky_relu(self.dense1(x))
        x = F.leaky_relu(self.dense2(x)) + x
        x = F.leaky_relu(self.dense3(x)) + x
        x = F.leaky_relu(self.dense4(x)) + x
        x = self.dense5(x)
        y = self._q_normalized(x)
        return y

    def _cal_loss(self, batch, y_hat, stage):
        assert len(self.loss_functions) > 0

        for i, (k, f) in enumerate(self.loss_functions):
            loss = f(batch, y_hat, self, stage)
            if i == 0:
                all_loss = k * loss
            else:
                all_loss = all_loss + k * loss

        self.log(f'{stage}/normal_loss', all_loss)
        return all_loss

    def training_step(self, batch, batch_idx):
        x, y = batch
        x_in = x[:, INPUT_ID2]
        y_hat = self.forward(x_in.view(x.size(0), -1))
        return self._cal_loss(batch, y_hat, 'train')

    def validation_step(self, batch, batch_idx):
        x, y = batch
        x_in = x[:, INPUT_ID2]
        y_hat = self.forward(x_in.view(x.size(0), -1))
        self._cal_loss(batch, y_hat, 'val')

    def configure_optimizers(self):
        """
        https://pytorch-lightning.readthedocs.io/en/latest/common/optimizers.html?highlight=lr_scheduler#learning-rate-scheduling
        :return:
        """
        optimizer = Adam(self.parameters(), lr=self.lr, weight_decay=self.weight_decay)
        # optimizer = AdamW(self.parameters(), lr=self.lr, weight_decay=self.weight_decay)
        if self.use_lr_step:
            scheduler = lr_scheduler.StepLR(optimizer, step_size=self.lr_step_size, gamma=self.lr_gamma)
            return [optimizer], [scheduler]
        else:
            return optimizer

    @staticmethod
    def add_model_specific_args(parent_parser):
        parser = parent_parser.add_argument_group('NaiveDNN')
        parser.add_argument('--restored_model_path', type=str, default='')
        parser.add_argument('--lr', type=float, default=1e-3)
        parser.add_argument('--use_lr_step', type=int, default=DEFAULT_USE_LR_STEP)
        parser.add_argument('--lr_step_size', type=float, default=DEFAULT_LR_STEP_SIZE)
        parser.add_argument('--lr_gamma', type=float, default=DEFAULT_LR_GAMMA)
        parser.add_argument('--weight_decay', type=float, default=DEFAULT_WEIGHT_DECAY)


def get_model(Model, dict_args):
    restored_model_path = dict_args['restored_model_path']
    if restored_model_path and os.path.exists(restored_model_path):
        print(f'Restoring model from path "{restored_model_path}" ...')
        model = Model.load_from_checkpoint(restored_model_path, **dict_args)
    else:
        print(f'Training model from scratch...')
        model = Model(**dict_args)
    return model


def get_checkpoint_list():
    def create_normal_checkpoint(stage, num):
        return ModelCheckpoint(
            monitor=f'{stage}/normal_loss',
            filename='epoch{epoch:02d}-' + stage + '-normal_loss{' + stage + '/normal_loss:.6f}',
            save_top_k=num,
            mode='min',
            auto_insert_metric_name=False
        )

    checkpoint_train = create_normal_checkpoint('train', 2)
    checkpoint_val = create_normal_checkpoint('val', 4)
    lr_monitor = LearningRateMonitor(logging_interval='epoch')
    checkpoint_callback_list = [checkpoint_train, checkpoint_val, lr_monitor]
    return checkpoint_callback_list


def get_train_config(Model, dict_args):
    model = get_model(Model, dict_args)
    dataloader = DataModule(**dict_args)
    checkpoint_callback_list = get_checkpoint_list()
    return model, dataloader, checkpoint_callback_list
