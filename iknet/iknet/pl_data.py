# -*- coding: utf-8 -*-
"""
@File  : pl_data.py
@Author: Yulong He
@Date  : 2021/11/11 10:03
@Desc  : 
"""
import numpy as np
import torch
import pytorch_lightning as pl
from torch.utils.data import Dataset, DataLoader, random_split
from iknet.config import IN_TRAIN_PATH, IN_TEST_PATH, RATIO_OF_TRAIN_AND_VALID, NUM_WORKERS
from sklearn import preprocessing
from py_anim.animation_ext import Animation
from py_anim.processing.files_operation import walk_dir
from py_anim.rotation import compute_ortho6d_from_rotation_matrix


class Data(Dataset):
    def __init__(self, train=True, scaler_transform=True, **kwargs):
        super().__init__()
        self.train = train

        if train:
            features = []
            targets = []

            def process(path, name):
                anim = Animation.from_bvh_filename(path)
                print(f'Processing {name}, frames: {anim.frame_num}')
                xyz = anim.forward_kinematics(world_reference=False)
                mat = anim.rotations.transforms()
                ortho6d = compute_ortho6d_from_rotation_matrix(mat)
                features.append(xyz)
                targets.append(ortho6d)

            walk_dir(IN_TRAIN_PATH, '.bvh', process)
            features = np.concatenate(features, axis=0).astype(np.float32)
            targets = np.concatenate(targets, axis=0).astype(np.float32)
            self.targets = torch.from_numpy(targets)
        else:
            raise RuntimeError('Not implemented!')
        if scaler_transform:
            self.scaler = None
            features = self.normalized(features)
        self.features = torch.from_numpy(features)
        self.sample_num = len(self.features)

    def __len__(self):
        return self.sample_num

    def __getitem__(self, i):
        if self.train:
            return self.features[i], self.targets[i]
        else:
            return self.features[i]

    def normalized(self, data: np.ndarray):
        # self.scaler = preprocessing.StandardScaler()
        B, J, D = data.shape
        data = data.reshape((B, -1))
        self.scaler = preprocessing.MinMaxScaler()
        self.scaler.fit(data)
        data = self.scaler.transform(data)
        return data.reshape((B, J, D))


class DataModule(pl.LightningDataModule):
    def __init__(self, batch_size: int = 32, **kwargs):
        super().__init__()
        self.batch_size = batch_size
        self.kwargs = kwargs
        self.train = None
        self.val = None
        self.test = None

    def prepare_data(self):
        """
        Use this method to do things that might write to disk
        or that need to be done only from a single process in distributed settings.
        :return:
        """
        pass

    def setup(self, stage: str = ''):
        if stage == 'fit' or stage == '':
            data_full = Data(train=True, **self.kwargs)
            train_size = int(len(data_full) * RATIO_OF_TRAIN_AND_VALID)
            valid_size = len(data_full) - train_size
            self.train, self.val = random_split(data_full, [train_size, valid_size])
        if stage == 'test' or stage == '':
            self.test = Data(train=False, **self.kwargs)

    def train_dataloader(self):
        return DataLoader(self.train, batch_size=self.batch_size, num_workers=NUM_WORKERS, shuffle=True)

    def val_dataloader(self):
        return DataLoader(self.val, batch_size=self.batch_size, num_workers=NUM_WORKERS, shuffle=True)

    def test_dataloader(self):
        return DataLoader(self.test, batch_size=self.batch_size, num_workers=NUM_WORKERS, shuffle=False)

    def predict_dataloader(self):
        return DataLoader(self.test, batch_size=self.batch_size, num_workers=NUM_WORKERS, shuffle=False)


if __name__ == '__main__':
    print('Debugging...')
    Data(train=True)
