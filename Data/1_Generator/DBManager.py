import os
import sqlite3
from settings import *

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

def DBfill():
    files = os.listdir(PATH_TO_ENV_FOLDER)
    files.remove(DATABASE_NAME)
    sql_script = ""
    for file in files:
        sql_script += f"INSERT INTO EnvConfig VALUES (NULL, '{file}', NULL)\n"
    execute_script(sql_script)

if __name__ == "__main__":
    DBsetup()
    DBfill()