; This example program checks if the input string is a binary palindrome.
; Input: a string of 0's and 1's, e.g. '11x111=11111'

; the finite set of states
#Q={copy_a,copy_b,cmp_a,cmp_b,accept,accept2,accept3,accept4,halt_accept,reject_erase,reject,reject2,reject3,reject4,reject5,halt_reject}

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
copy_a x__ ___ *** copy_b
copy_a =__ ___ r** reject_erase
copy_b 1__ __1 r*r copy_a
copy_b x__ ___ r** reject_erase
copy_b =__ ___ r** sub_init

sub_init 1__ 1__ *ll sub_ex
sub_ex 111 _11 r*l sub_ex
sub_ex 11_ 1__ **r sub_mr
sub_ex 1_1 1_1 *** reject_erase
sub_ex _11 _11 *** reject_erase
sub_ex _1_ _1_ *** accept
sub_mr 1_1 1_1 **r sub_mr
sub_mr 1__ 1__ *** sub_init

; State accept*: write 'true' on 1st tape
accept ___ t__ r** accept2
accept2 ___ r__ r** accept3
accept3 ___ u__ r** accept4
accept4 ___ e__ *** halt_accept

; State reject*: write 'false' on 1st tape
reject_erase a__ ___ r** reject_erase
reject_erase b__ ___ r** reject_erase
reject_erase _a_ ___ *l* reject_erase
reject_erase __b ___ **l reject_erase
reject_erase _ab ___ *rl reject_erase
reject_erase aa_ ___ rl* reject_erase
reject_erase a_b ___ r*l reject_erase
reject_erase ba_ ___ rl* reject_erase
reject_erase b_b ___ r*l reject_erase
reject_erase aab ___ rll reject_erase
reject_erase bab ___ rll reject_erase
reject_erase ___ ___ *** reject
reject ___ f__ r** reject2
reject2 ___ a__ r** reject3
reject3 ___ l__ r** reject4
reject4 ___ s__ r** reject5
reject5 ___ e__ *** halt_reject

