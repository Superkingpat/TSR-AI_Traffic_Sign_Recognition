import os
import sqlite3
from settings import *

def execute(sql_script, param_list = []): #custom SQL implementation from FMP project
    conn = sqlite3.connect(os.path.join(PATH_TO_ENV_FOLDER,DATABASE_NAME))
    #print("BASE")
    cursor = conn.cursor()
    lines = sql_script.split('\n')
    error = False

    var_dict = {}

    for line in lines:
        print(line)
        var_name = ""
        sql_querry = ""
        
        if error:
            line = line.replace("--", "")
        error = False
        
        if "--" in line[0:2]:
            param_list = param_list[line.count("?"):]
            continue

        if ":=" in line:
            var_name,sql_querry = line.split(":=")
        else:
            sql_querry = line

        # execute     
        if sql_querry != "":
            temp_param = []
            if ":" in sql_querry:
                for i in range(0,len(sql_querry)-1):
                    if sql_querry[i] == ":": 
                        var = ""
                        while sql_querry[i] != " " and sql_querry[i] != "," and i < len(sql_querry):
                            var += sql_querry[i]
                            i += 1
                        try: temp_param.append(var_dict[var.strip()])
                        except KeyError:
                            raise ChildProcessError("Can't find variable " + var)
                        except Exception as e: 
                            print("PARAMS: " + str(e))
                    elif sql_querry[i] == "?":
                        temp_param.append(param_list.pop(0))
            else:
                temp_param = param_list[0:line.count("?")]
                param_list = param_list[line.count("?"):]
            
            try: 
                #print(tuple(temp_param))
                cursor.execute(sql_querry, tuple(temp_param))
                if var_name != "": 
                    if "select" in line.lower(): var_dict[var_name.strip()] = cursor.fetchall()[0][0]
                    else: var_dict[var_name.strip()] = cursor.lastrowid
            except Exception as e: 
                error = True
    conn.commit()
    cursor.close()
    conn.close()
    return var_dict
    #print("BASE DONE!")

def execute_script(sql_script):
    conn = sqlite3.connect(os.path.join(PATH_TO_ENV_FOLDER,DATABASE_NAME))
    cursor = conn.cursor()
    for sql_statement in sql_script.split(';'):
        try: cursor.execute(sql_statement)
        except Exception as e: print(e)
    conn.commit()
    cursor.close()
    conn.close()

def DBsetup():
    with open(DIR+"/DBSetup.sql", 'r') as file:
        execute_script(file.read())
    DBfill()

def DBfill():
    files = os.listdir(PATH_TO_ENV_FOLDER)
    files.remove(DATABASE_NAME)
    sql_script = ""
    param_list = []
    for file in files: # Don't break unique constraint
        sql_script += f":a := SELECT fileName FROM EnvConfig WHERE fileName = ?;\n"
        param_list.append(file)
        sql_script += f"--:a := INSERT INTO EnvConfig VALUES (NULL, ?, NULL);\n"
        param_list.append(file)
    execute(sql_script, param_list)

if __name__ == "__main__":
    DBfill()