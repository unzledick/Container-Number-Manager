import os
import sys
from sys import platform as _platform

import json

m_server = dict({'CPU' : {'CPU load'},
                 'Memory' : {'Memory'},
                 'Network' : {'Network'},
                 'Disk' : {'Disk'}
                 })
m_application = dict({'performance' : {'Response time'},
                'hardware_resource' : {'CpuUsage', 'MemoryUsage'},
                'heap_memory' : {'used', 'max'}, 
                'connection_pool' : {'ActiveCount', 'AvailableCount', 'AverageCreationTime'}, 
                'cache' : {'query-cache-hit-count', 'query-execution-max-time', 'second-level-cache-hit-count'},
                'thread_pool' : {'pool-available-count', 'pool-create-count', 'pool-max-size'}
                })

class treeNode:
    def __init__ (self, type, root):
        self.type = type
        self.root = root

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
        print('\t',r,'\t',obj[r])

def get_rule_value(metric, existing_rule_set):
        
    metric_msg = '\t' + metric + '['  
    if metric in existing_rule_set:
        metric_msg += str(existing_rule_set[metric])
    metric_msg += ']:'

    input_val = input(metric_msg)
    try:
        val = float(input_val)
    except ValueError:
        if(input_val is ''):    # no change
            if metric in existing_rule_set:
                val = existing_rule_set[metric]
            else:
                val = -1        
    return val

def mod_rules_in_obj(tree, item):
    print('\nInput a positive value to update, 0 to delete, press Enter to skip\n')

    if tree.type is 'Server':
        m_set = m_server
    else:
        m_set = m_application

    for category in iter(m_set):
        print('**'+category+'**')
        for metric in iter(m_set[category]):                    
            val = get_rule_value(metric, tree.root[item])                    
            if(val > 0):
                tree.root[item].update({metric:val})
            elif(val == 0):
                tree.root[item].pop(metric, -1)

def mod_obj(tree, obj_name):
    clear_screen()
           
    if(type(tree.root[obj_name]) is not dict):
        tree.root[obj_name] = dict()

    if(len(tree.root[obj_name]) > 0):    
        show_curr_rules(tree.root[obj_name])
    else:
        print('There is no rule for', obj_name)
    
    if input('Modify?(Y/[N])').capitalize() == 'Y':
        mod_rules_in_obj(tree, obj_name)

def mod_obj_in_tree(tree):    
    while(True):
        obj_name = input('enter name:')

        if(obj_name is ''):     # empty input, return
            break
        if(obj_name not in tree.root):
            print('Does not existed')
        else:            
            mod_obj(tree, obj_name)
            break;

def add_obj_to_tree(root):
    obj_name = input('enter name:')

    if(obj_name is ''):         # empty input, return
        return

    if(obj_name in root):
        print('Already existed')
    else:      
        root.update({obj_name:''})

def show_sub_tree(tree):
    print('Current Contents in', tree.type)
    
    for obj in iter(tree.root):
        print('\t',obj)
    print()

def modify_sub_tree(tree):

    if(type(tree.root) is not dict):
        tree.root = dict()
    
    while(True):
        menu_2_msg = '1) Add\n'
        if(len(tree.root) > 0):
            menu_2_msg += '2) Modify\n3) Delete\n'        
        menu_2_msg += 'b) Back\n'       
    
        clear_screen()
        show_sub_tree(tree)
        option = input(menu_2_msg)

        if(option == '1'):
            add_obj_to_tree(tree.root)
        elif(option == '2'):            
            mod_obj_in_tree(tree)
        elif(option == '3'):
            del_obj_in_tree(tree.root)
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

root_server = treeNode('Server', rule_tree['Server'])
root_application = treeNode('Application', rule_tree['Application'])

while(True):
    clear_screen() 
    user_option = input(menu_1_msg)

    if(user_option == '1'):        
        modify_sub_tree(root_server)
    elif(user_option == '2'):                
        modify_sub_tree(root_application)
    elif(user_option == '3'):
        rule_tree.update({root_server.type:root_server.root})
        rule_tree.update({root_application.type:root_application.root})
        with open('SLA.json', 'w') as outfile:
            json.dump(rule_tree, outfile, sort_keys = True, indent=4)
        break;
    elif(user_option == '4'):
        break
    else:
        continue
