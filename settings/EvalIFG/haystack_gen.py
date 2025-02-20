num_other_paths = 99
other_path_fitness = 0.1 
path_length = 7

print('add, start, 1')
print('')
print('add, B, 9')
print('connect, start, B, ' + str(path_length))
print('')
print('add, finish, 10 ')
print('connect, B, finish, 0')
print('')

for i in range(num_other_paths):
  node_name = 'N' + str(i + 1)
  print('add, ' + node_name + ', ' + str(other_path_fitness))
  print('connect, start, ' + node_name + ', ' + str(path_length))
  print('connect, ' + node_name + ', finish, 0')

