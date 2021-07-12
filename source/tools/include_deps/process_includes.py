input_filename = 'include_list.txt'
dot_filename = 'output.dot'

from graphviz import Digraph

D = {}
included_list = []

with open(input_filename, 'r') as in_fp:
    for line in in_fp:
        line = line.strip()
        line_parts = line.split()
        filename = line_parts[0].split('#')[0].strip(':').strip()
        scope_list = ['source']
        filename_parts = filename.split('/')
        for idx in range(0, len(filename_parts) - 1):
            scope_list.append(filename_parts[idx])
        filename = filename_parts[-1]
        orig_scope_list = [*scope_list]
        include_path = line_parts[1].strip('"')
        while len(include_path) > 3 and include_path[0:3] == '../':
            scope_list = scope_list[:-1]
            include_path = include_path[3:]
        include_parts = include_path.split('/')
        for idx in range(0, len(include_parts) - 1):
            scope_list.append(include_parts[idx])
        include = include_parts[-1]
        full_source_path = '/'.join(orig_scope_list) + '/' + filename
        full_include_path = '/'.join(scope_list) + '/' + include
        if full_source_path not in D.keys():
            D[full_source_path] = []
        D[full_source_path].append(full_include_path)
        if full_include_path not in included_list:
            included_list.append(full_include_path)
        #print(full_source_path)

print('Level 0')
base_path_list = []
for path in included_list:
    if path not in D.keys():
        print('\t', path)
        base_path_list.append(path)

def get_level(path, cur_level = 0):
    if path not in D.keys():
        return cur_level
    include_vals = []
    for include in D[path]:
        include_vals.append(get_level(include, cur_level + 1))
    return max(include_vals)

        
level_list = []

for source_path in D.keys():
    level = get_level(source_path)
    while len(level_list) <= level:
        level_list.append([])
    level_list[level].append(source_path)

for level in range(1, len(level_list)):
    print('Level', level)
    for path in level_list[level]:
        print('\t', path)

node_D = {}
cur_idx = 0

graph = Digraph()
with graph.subgraph() as base_layer:
    base_layer.attr(rank='same')
    for path in base_path_list:
        if path not in node_D.keys():
            node_D[path] = str(cur_idx)
            cur_idx += 1
            base_layer.node(node_D[path], path, shape='house', pos='0,0')
for level in range(1, len(level_list)):
    with graph.subgraph() as layer:
        layer.attr(rank='same')
        for path in level_list[level]:
            if path not in node_D.keys():
                node_D[path] = str(cur_idx)
                cur_idx += 1
                layer.node(node_D[path], path)
            for include_path in D[path]:
                if include_path not in node_D.keys():
                    node_D[include_path] = str(cur_idx)
                    cur_idx += 1
                    layer.node(node_D[include_path], include_path)
                graph.edge(node_D[path], node_D[include_path])
graph.render('dependencies')


#with open(dot_filename, 'w') as dot_fp:
#    dot_fp.write('digraph includes {\n')
#    for source_path in D.keys():
#        for include_path in D[source_path]:
#            dot_fp.write(source_path + ' -> ' + include_path + '\n')
#
#    dot_fp.write('}')
