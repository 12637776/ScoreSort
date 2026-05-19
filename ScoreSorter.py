# 按照CSV成绩划分文件夹
# Divide folders by CSV.

import os
import shutil
import tkinter as tk
from tkinter import filedialog


def mkdir(a):  # 制作文件夹
    if os.path.exists(a):  # 判断是否已经存在文件夹
        pass
    else:
        os.mkdir(a)  # 根据提供的路径创建文件夹


root = tk.Tk()
root.withdraw()

path = filedialog.askdirectory()
score = filedialog.askopenfilename()
st = '课程设计成绩'
s1 = '60分以下'
s2 = '60-69分'
s3 = '70-79分'
s4 = '80-89分'
s5 = '90-100分'
se = '作业'
p_s1 = path + '//' + st + s1 + se
p_s2 = path + '//' + st + s2 + se
p_s3 = path + '//' + st + s3 + se
p_s4 = path + '//' + st + s4 + se
p_s5 = path + '//' + st + s5 + se
mkdir(p_s1)
mkdir(p_s2)
mkdir(p_s3)
mkdir(p_s4)
mkdir(p_s5)

with open(score, 'r', encoding="gbk") as t:
    for line in t:
        line = line.replace('\n', '')
        if line == '学号,姓名,成绩':
            pass
        else:
            # print('Line: ', line)
            symbol_1 = line.find(',')
            # print(symbol_1)
            stu_sn = line[:symbol_1]
            # print('学号: ', stu_sn)
            name_scr = line[symbol_1 + 1:]
            symbol_2 = name_scr.find(',')
            # print(symbol_2)
            stu_name = name_scr[:symbol_2]
            # print('姓名: ', stu_name)
            stu_scr = name_scr[symbol_2 + 1:]
            # print('成绩: ', stu_scr)
            s = int(stu_scr)
            stu_sn_name = stu_sn + ' ' + stu_name
            stu_path1 = path + '//' + stu_sn_name
            if os.path.exists(stu_path1):
                if s >= 90:
                    #  print('【90~100】')
                    stu_path2 = p_s5 + '//' + stu_sn_name
                elif s >= 80:
                    #   print('【80~89】')
                    stu_path2 = p_s4 + '//' + stu_sn_name
                elif s >= 70:
                    #  print('【70~79】')
                    stu_path2 = p_s3 + '//' + stu_sn_name
                elif s >= 60:
                    # print('【60~69】')
                    stu_path2 = p_s2 + '//' + stu_sn_name
                else:
                    # print('【60以下】')
                    stu_path2 = p_s1 + '//' + stu_sn_name
                shutil.move(stu_path1, stu_path2)
            else:
                pass
                # print(stu_sn_name, '缺失')

files_main = os.listdir(path)  # 读入文件夹
for file1 in files_main:
    old_path = path + '//' + file1
    f1e = os.path.splitext(file1)[1]
    f1ex = len(f1e)
    new_path = path + '//其他//' + file1
    mkdir(path + '//其他//')
    if file1.find('(') > 0:
        # print('Type - I: ', new_path)
        shutil.move(old_path, new_path)
    elif f1ex > 0:
        shutil.move(old_path, new_path)
        # print('Type - II: ', new_path)
    else:
        pass

# Powered by Pengo
# Start from 2023/7/7 17:23
