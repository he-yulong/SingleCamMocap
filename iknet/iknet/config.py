# -*- coding: utf-8 -*-
"""
@File  : config.py
@Author: Yulong He
@Date  : 2021/11/11 10:04
@Desc  : 
"""
import os
from py_anim.format.skeleton import vicon_skl33_v1

API_MODEL_PATH = os.path.join(os.path.dirname(__file__), 'assets/epoch34-val-normal_loss5.024345.ckpt')
IN_TRAIN_PATH = 'D:/OneDrive/dataset/tweening/Turtle/Process_BVH_ZYX_Init_FPS25/'
# IN_TRAIN_PATH = 'D:/OneDrive/dataset/tweening/Turtle/Process_debug/'
IN_TEST_PATH = ''

# Dataset Configurations
RATIO_OF_TRAIN_AND_VALID = 0.95
NUM_WORKERS = 1

# Model Configurations
INPUT_XYZ_NAMES = [
    'Neck1', 'Head',
    'RightArm', 'RightForeArm', 'RightHand',
    'LeftArm', 'LeftForeArm', 'LeftHand',
    'RightUpLeg', 'RightLeg', 'RightFoot', 'RightForeFoot', 'RightToeBaseEnd',
    'LeftUpLeg', 'LeftLeg', 'LeftFoot', 'LeftForeFoot', 'LeftToeBaseEnd',
]
INPUT_XYZ_NAMES2 = [
    'Neck1', 'Head',
    'RightArm', 'RightForeArm', 'RightHand', 'RightHandThumb', 'RightHandMiddle',
    'LeftArm', 'LeftForeArm', 'LeftHand', 'LeftHandThumb', 'LeftHandMiddle',
    'RightUpLeg', 'RightLeg', 'RightFoot', 'RightForeFoot', 'RightToeBaseEnd',
    'LeftUpLeg', 'LeftLeg', 'LeftFoot', 'LeftForeFoot', 'LeftToeBaseEnd',
]
INPUT_ID = [vicon_skl33_v1.joint_names.index(name) for name in INPUT_XYZ_NAMES]
INPUT_ID2 = [vicon_skl33_v1.joint_names.index(name) for name in INPUT_XYZ_NAMES2]
DEFAULT_BATCH_SIZE = 64
DEFULAT_LR = 1e-4
DEFAULT_USE_LR_STEP = 0
DEFAULT_LR_STEP_SIZE = 10
DEFAULT_LR_GAMMA = 0.9
DEFAULT_WEIGHT_DECAY = 1e-5
ROT_JNUM = 26
ALL_JNUM = 33
