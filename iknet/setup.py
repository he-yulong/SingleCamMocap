# -*- coding: utf-8 -*-
"""
@File  : setup.py
@Author: Yulong He
@Date  : 2021/11/5 9:40
@Desc  : 
"""
import setuptools

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setuptools.setup(
    name="iknet",
    version="1.0.0",
    author="Yulong He",
    author_email="1183851628@qq.com",
    description="character animation for iknet.",
    long_description_content_type="text/markdown",
    url="https://github.com/Noluye",
    packages=setuptools.find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    include_package_data=True
)
