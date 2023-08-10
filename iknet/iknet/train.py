# -*- coding: utf-8 -*-
"""
@File  : train.py
@Author: Yulong He
@Date  : 2021/11/11 10:58
@Desc  : 
"""
from iknet.model.pl_model_v1 import NaiveDNN, NaiveDNN2

if __name__ == '__main__':
    from argparse import ArgumentParser
    from pytorch_lightning import Trainer
    from iknet.config import DEFAULT_BATCH_SIZE

    parser = ArgumentParser()
    # add PROGRAM level args
    parser.add_argument('--model_name', type=str, default='v1',
                        help='v1,v2,...')  # figure out which model to use
    parser.add_argument('--batch_size', type=int, default=DEFAULT_BATCH_SIZE)
    parser.add_argument('--scaler_transform', type=int, default=0, help='data standardization')
    # -------------------------------------------------
    # add Trainer args
    parser = Trainer.add_argparse_args(parser)


    # -------------------------------------------------
    # add model specific args
    def choose_model_to_add_specific_args(parser):
        args, _ = parser.parse_known_args()
        # let the model add what it wants
        if args.model_name == 'v1':
            NaiveDNN.add_model_specific_args(parser)
        elif args.model_name == 'v2':
            NaiveDNN2.add_model_specific_args(parser)
        elif args.model_name == 'v3':
            raise RuntimeError('No current model specified!')
        else:
            raise RuntimeError('No current model specified!')
        return parser


    parser = choose_model_to_add_specific_args(parser)
    # -------------------------------------------------
    # parser.print_help()
    args = parser.parse_args()
    # -------------------------------------------------
    # train
    if args.model_name in ['v1', 'v2', 'v3']:
        from iknet.model.pl_model_v1 import get_train_config

        if args.model_name == 'v1':
            Model = NaiveDNN
        elif args.model_name == 'v2':
            Model = NaiveDNN2
        elif args.model_name == 'v3':
            raise RuntimeError('No current model specified!')
        else:
            raise RuntimeError('No current model specified!')
    elif args.model_name == 'v200':
        raise RuntimeError('No current model specified!')
    else:
        raise RuntimeError('No current model specified!')
    dict_args = vars(args)
    model, dataloader, checkpoint_callback_list = get_train_config(Model, dict_args)
    # -------------------------------------------------
    # init the trainer
    trainer = Trainer.from_argparse_args(args, callbacks=checkpoint_callback_list)
    # -------------------------------------------------
    trainer.fit(model, dataloader)
