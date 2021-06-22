import re

# Configuration options
source_dir = '../../source/'                       # Where is the source directory?
module_filename = source_dir + 'modules.hpp'    # Name of the module file?
output_filename = './mabe.vim'                  # Where to save the output of this script?
base_script_filename = 'base.vim'               # Where is the base syntax file located?
# Names of built in types (must be included by hand for now)
builtin_type_list = ['Value', 'Population']

# Look through module.hpp to find all included files
included_filename_list = []
with open(module_filename, 'r') as in_fp:
    for line in in_fp:
        line = line.strip()
        if line == '': 
            continue
        line_parts = line.split()
        if len(line_parts) == 2 and line_parts[0] == '#include': # format: #include "file.hpp"
            included_filename_list.append(line_parts[1].strip('"'))

# Look through included files to find classes that are derived from Module or the like
regex_prog = re.compile(\
        '(class|struct)\s+(\w+)\s*:\s*(public|protected|private)\s+(Module|OrganismTemplate)')
module_list = []
for filename in included_filename_list:
    with open(source_dir + filename) as in_fp:
        file_contents = in_fp.read()
        match_results = regex_prog.search(file_contents)
        if match_results != None: # Match!
            module_name = match_results[2] # Third group to get class name (don't forget 0 = all!)
            module_list.append(module_name)

print('Found', len(module_list), 'modules in', len(included_filename_list), 'included files:')
print(' '.join(module_list))
print('Adding', len(builtin_type_list), 'built-in type(s)!')

# Join module list + built-in list
type_list = module_list + builtin_type_list

# Load in base script and add our type list to it!  
with open(base_script_filename, 'r') as in_fp:
    base_script = in_fp.read()
script = base_script.replace('<<TYPES>>', ' '.join(type_list))

# Write output file!
with open(output_filename, 'w') as out_fp: 
    out_fp.write(script)

print('Generation finished! File saved to: ' + output_filename + '!')
print('Note: this output is not guaranteed to be perfect! If you find an error feel free to let me (Austin) know!')

