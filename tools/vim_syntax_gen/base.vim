" Vim syntax file
" Language: MABE Config Language 
" Maintainer: Austin J Ferguson
" Latest Revision: 21 June 2021
" Heavily draws from
" https://vim.fandom.com/wiki/Creating_your_own_syntax_files (accessed 2021-06-21)

if exists("b:current_syntax")
  finish
endif

syn keyword mabeModuleName <<TYPES>> 
syn match mabeComment '//.*$'
syn match mabeFunction '@\w*'
syn region mabeBlock start="{" end="}" fold transparent
syn region mabeString start="\"" end="\""

" Regular int like number with - + or nothing in front
syn match mabeNumber '\d\+' 
syn match mabeNumber '[-+]\d\+' 

" Floating point number with decimal no E or e (+,-)
syn match mabeNumber '\d\+\.\d*' 
syn match mabeNumber '[-+]\d\+\.\d*' 

"" Floating point like number with E and no decimal point (+,-)
"syn match mabeNumber '[-+]\=\d[[:digit:]]*[eE][\-+]\=\d\+' 
"syn match mabeNumber '\d[[:digit:]]*[eE][\-+]\=\d\+' 
"
"" Floating point like number with E and decimal point (+,-)
"syn match mabeNumber '[-+]\=\d[[:digit:]]*\.\d*[eE][\-+]\=\d\+' 
"syn match mabeNumber '\d[[:digit:]]*\.\d*[eE][\-+]\=\d\+' 

let b:current_syntax = "mabe"

hi def link mabeModuleName Type
hi def link mabeComment Comment
hi def link mabeFunction Statement
hi def link mabeNumber Constant
hi def link mabeString Constant

