def remove_comments(line):
    line_parts = line.split('//')
    return line_parts[0].strip()

def parse_config(filename):
    module_list = []
    conn_list = []
    with open(filename, 'r') as in_fp:
        for line in in_fp:
           line = remove_comments(line)
           if line == '':
               continue
           print(line)
    return [module_list, conn_list]

