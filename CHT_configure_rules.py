import os
import sys
from sys import platform as _platform

import json

def clear_screen():
    if _platform == "linux" or _platform == "linux2":
        os.system('clear')
    elif _platform == "win32":
        os.system('cls')
    #else:
    #   do nothing

def del_obj_in_tree(root):
    while(True):
        obj_name = input('Enter the name to delete:')

        if(obj_name is ''):     # empty input, return
            break
        if(root.pop(obj_name, -1) == -1):
            print('[remove fail]', obj_name, 'does not exist')
        else:                   # delete successfully
            break


def mod_obj(obj_name):
    print('omg')

def add_obj_to_tree(root):
    input_name = input('enter name:')

    if(input_name is ''):       # empty input, return
        return

    if(input_name in root):
        print('Already existed')
    else:
        # [TODO] call modify 
        # result = mod_obj(input_name)
        # root.update({input_name:result})
        root.update({input_name:''})

def show_sub_tree(tName):
    print('Current Contents in', tName)
    
    for obj in iter(rule_tree[tName]):
        print('\t',obj)

def modify_sub_tree(tName):

    msg_2 = '1) Add\n'
    if(tName in rule_tree):
        msg_2 += '2) Modify\n3) Delete\n'        
    msg_2 += 'b) Back\n'       

    while(True):
        clear_screen()
        show_sub_tree(tName)
        option = input(msg_2)

        if(option == '1'):
            add_obj_to_tree(rule_tree[tName])
        elif(option == '2'):
            print('2')
        elif(option == '3'):
            del_obj_in_tree(rule_tree[tName])
        elif(option == 'b'):
            break;
        else:
            continue;

# start here!
#print('This is a Python script that helps you configrues the server and application rules.')

try:
    f = open('SLA.json', 'r+')
except IOError:
    f = open('SLA.json', 'w')
    print('New configure file has been created.')

rule_tree = json.load(f)

f.close()

msg_1 =  '1) Modify Server Rules\n'
msg_1 += '2) Modify Application Rules\n'
msg_1 += '3) Save the current settings\n'
msg_1 += '4) exit\n'
msg_1 += 'Please select your option:\n'

while(True):
    clear_screen() 
    user_option = input(msg_1)

    if(user_option == '1'):
        modify_sub_tree('Server')
    elif(user_option == '2'):
        modify_sub_tree('Application')
    elif(user_option == '3'):
        try:
            f = open('SLA.json', 'r+')
        except IOError:
            print('Something wrong QQ')
        f.close()
    elif(user_option == '4'):
        break
    else:
        continue

#json.dump();
