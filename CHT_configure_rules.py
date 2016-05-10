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

def show_curr_rules(obj):
    for r in iter(obj):
        print(r)
        pass

def mod_rules_in_obj(root, key):
    clear_screen()

    if(root is not ''):
        show_curr_rules(root)
    else:
        print('There is no rule for',key)

    if input('Modify?(Y/[N])').capitalize() is 'Y':
        # [TODO]
        # modify all rules
        pass

def mod_obj_in_tree(root):
    while(True):
        obj_name = input('enter name:')

        if(obj_name is ''):     # empty input, return
            break
        if(obj_name not in root):
            print('Does not existed')
        else:            
            mod_rules_in_obj(root[obj_name], obj_name)
            break;

def add_obj_to_tree(root):
    
    obj_name = input('enter name:')

    if(obj_name is ''):         # empty input, return
        return

    if(obj_name in root):
        print('Already existed')
    else:      
        root.update({obj_name:''})

def show_sub_tree(root, key):
    print('Current Contents in', key)
    
    for obj in iter(root[key]):
        print('\t',obj)
    print()

def modify_sub_tree(root, key):

    if(root[key] is ''):
        root.update({key:dict()})            

    while(True):
        menu_2_msg = '1) Add\n'
        if(len(root[key]) > 0):
            menu_2_msg += '2) Modify\n3) Delete\n'        
        menu_2_msg += 'b) Back\n'       
    
        clear_screen()
        show_sub_tree(root, key)
        option = input(menu_2_msg)

        if(option == '1'):
            add_obj_to_tree(root[key])
        elif(option == '2'):
            mod_obj_in_tree(root[key])
        elif(option == '3'):
            del_obj_in_tree(root[key])
        elif(option == 'b'):
            break;
        else:
            continue;

# start here!
try:
    f = open('SLA.json', 'r+')
except IOError:                 # file not exsits
    f = open('SLA.json', 'w+')
    json.dump({'Server':'', 'Application':''}, f, sort_keys = True, indent=4)    
    f.flush()
    f.seek(0)
 
try:
    rule_tree = json.load(f)
except ValueError:
    print('Input file is not in JSON format')
    f.close()
    sys.exit()

f.close()

menu_1_msg =  '1) Modify Server Rules\n'
menu_1_msg += '2) Modify Application Rules\n'
menu_1_msg += '3) Save and Exit\n'
menu_1_msg += '4) Exit without Saving\n\n'
menu_1_msg += 'Please select your option:\n'

while(True):
    clear_screen() 
    user_option = input(menu_1_msg)

    if(user_option == '1'):
        modify_sub_tree(rule_tree, 'Server')
    elif(user_option == '2'):        
        modify_sub_tree(rule_tree, 'Application')
    elif(user_option == '3'):
        with open('SLA.json', 'w') as outfile:
            json.dump(rule_tree, outfile, sort_keys = True, indent=4)
        break;
    elif(user_option == '4'):
        break
    else:
        continue
