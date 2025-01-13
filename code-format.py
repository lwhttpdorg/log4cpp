# encoding=UTF-8
import os

def fina_all_code_file(base: str):
    for root, _, files in os.walk(base):
        for file in files:
            if not file.startswith('.') and (file.endswith('.h') or file.endswith('.hpp') or file.endswith('.c') or file.endswith('.cpp')):
                yield f'{root}/{file}'

def clang_format(path: str):
    for code_file in fina_all_code_file(path):
        # clang-format
        command = f"clang-format -i {code_file}"
        print(command)
        os.system(command)

if __name__ == '__main__':
    current_path = os.getcwd()
    clang_format(current_path + '/src')
    clang_format(current_path + '/demo')
