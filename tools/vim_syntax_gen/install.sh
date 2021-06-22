#!/bin/bash

# Create necessary directories (and any leading directories)
mkdir -p ~/.vim/ftdetect
mkdir -p ~/.vim/syntax

# Run the script! 
echo "Generating syntax file..."
python3 generate_vim_syntax.py 

# Install! 
echo "Copying files to vim folder"
cp mabe.vim ~/.vim/syntax
cp detect.vim ~/.vim/ftdetect/mabe.vim
echo "Done!"
