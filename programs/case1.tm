; This example program checks if the input string is a binary palindrome.
; Input: a string of a's and b's, e.g. 'abbabba'

; the finite set of states
#Q={ copy_a , copy_b ,cmp_a,cmp_a1,cmp_b,accept,accept2,accept3,accept4,halt_accept,reject_erase,reject,reject2,reject3,reject4,reject5,halt_reject}

; the finite set of input symbols
#S = {a,b}

; the complete set of tape symbols
#G = {a,b,_,t,r,u,e,f,a,l,s}

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
copy_a a__ _a_ rr* copy_a
copy_a b__ __b r*r copy_b
copy_b b__ __b r*r copy_b
copy_b a__ a__ *** cmp_a

cmp_a a__ a__ *l* cmp_a1
cmp_a1 aa_ ___ rl* cmp_a1
cmp_a1 a__ ___ *** reject_erase
cmp_a1 ba_ b__ *** reject_erase
cmp_a1 b__ b__ **l cmp_b

cmp_b b_b ___ r*l cmp_b
cmp_b b__ ___ r** reject_erase
cmp_b a_b ___ r** reject_erase
cmp_b a__ ___ r** reject_erase
cmp_b ___ ___ *** accept

; State accept*: write 'true' on 1st tape
accept ___ t__ r** accept2
accept2 ___ r__ r** accept3
accept3 ___ u__ r** accept4
accept4 ___ e__ *** halt_accept

; State reject*: write 'false' on 1st tape
reject_erase a__ ___ r** reject_erase
reject_erase b__ ___ r** reject_erase
reject_erase aa_ ___ rl* reject_erase
reject_erase a_b ___ r*l reject_erase
reject_erase aab ___ r*l reject_erase
reject_erase ___ ___ *** reject
reject ___ f__ r** reject2
reject2 ___ a__ r** reject3
reject3 ___ l__ r** reject4
reject4 ___ s__ r** reject5
reject5 ___ e__ *** halt_reject

