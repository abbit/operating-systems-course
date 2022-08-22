#!/usr/bin/env python3

import sys
import os
import shutil
import argparse
from pathlib import Path

TEMPLATE_STR = '[[$]]'

parser = argparse.ArgumentParser(description='Creates a new C project with given name')
parser.add_argument('name', type=str, help='name of the project which will be created')

args = parser.parse_args()
project_name = args.name

shutil.copytree('template', project_name)

cwd = Path(project_name)
for file in cwd.iterdir():
    if (file.is_dir()):
        continue

    with file.open() as f: lines = f.readlines()
    with file.open(mode='w') as f:
        for line in lines:
            f.write(line.replace(TEMPLATE_STR, project_name))

    if(TEMPLATE_STR in file.name):
        file.rename(cwd.name + '/' + file.name.replace(TEMPLATE_STR, project_name))
