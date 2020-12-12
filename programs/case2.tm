; This example program checks if the input string is a binary palindrome.
; Input: a string of 0's and 1's, e.g. '11x111=11111'

; the finite set of states
#Q={copy_a,copy_b,res_fmt_chk,res_fmt_chk_rej_ml,res_fmt_chk_ml,sub_init,sub_ac_chk1,sub_ac_chk2,sub_ex,sub_mr,accept,accept2,accept3,accept4,halt_accept,reject,reject2,reject3,reject4,reject5,halt_reject}

; the finite set of input symbols
#S = {0,1,x,=}

; the complete set of tape symbols
#G = {x,=,0,1,_,t,r,u,e,f,a,l,s}

; the start state
#q0 = copy_a

; the blank symbol
#B = _

; the set of final states
#F = {halt_accept}

; the number of tapes
#N = 3

; the transition functions

; copy input
copy_a 1__ _1_ rr* copy_a
copy_a x__ ___ r** copy_b
copy_a =__ =__ *** reject
copy_b 1__ __1 r*r copy_b
copy_b x__ x__ *** reject
copy_b =__ ___ r** res_fmt_chk

res_fmt_chk 1__ 1__ r** res_fmt_chk 
res_fmt_chk =__ =__ *** res_fmt_chk_rej_ml
res_fmt_chk x__ x__ *** res_fmt_chk_rej_ml
res_fmt_chk ___ ___ l** res_fmt_chk_ml
res_fmt_chk_ml 1__ 1__ l** res_fmt_chk_ml
res_fmt_chk_ml ___ ___ r** sub_init
res_fmt_chk_rej_ml 1__ 1__ l** res_fmt_chk_rej_ml
res_fmt_chk_rej_ml =__ =__ l** res_fmt_chk_rej_ml
res_fmt_chk_rej_ml x__ x__ l** res_fmt_chk_rej_ml
res_fmt_chk_rej_ml ___ ___ r** reject

sub_init ___ 1__ *** reject
sub_init 1__ 1__ *ll sub_ac_chk1
sub_ac_chk1 1__ 1__ *** accept
sub_ac_chk1 1_1 1__ *** reject
sub_ac_chk1 111 111 *** sub_ex
sub_ac_chk1 11_ 1__ *** reject

sub_ex 111 _11 r*l sub_ex
sub_ex _1_ ___ *l* sub_ac_chk2
sub_ex _11 ___ *** reject
sub_ex 11_ 1__ **r sub_mr

sub_mr 1_1 1_1 **r sub_mr
sub_mr 1__ 1__ *** sub_init

sub_ac_chk2 _1_ ___ *** reject
sub_ac_chk2 ___ ___ *** accept

; State accept*: write 'true' on 1st tape
accept ___ t__ r** accept2
accept2 ___ r__ r** accept3
accept3 ___ u__ r** accept4
accept4 ___ e__ *** halt_accept

; State reject*: write 'false' on 1st tape
reject 1__ ___ r** reject
reject =__ ___ r** reject
reject x__ ___ r** reject
reject ___ f__ r** reject2
reject2 ___ a__ r** reject3
reject3 ___ l__ r** reject4
reject4 ___ s__ r** reject5
reject5 ___ e__ *** halt_reject

