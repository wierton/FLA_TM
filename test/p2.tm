; This example program checks if the input string is a binary palindrome.
; Input: a string of 0's and 1's, e.g. '1001001'

; the finite set of states
#Q = {q0,q1,q2,q3,q4,q5,q6,acc,rej}

; the finite set of input symbols
#S = {a,b}

; the complete set of tape symbols
#G = {a,b,_}

; the start state
#q0 = q0

; the blank symbol
#B = _

; the set of final states
#F = {acc}

; the number of tapes
#N = 1

; the transition functions

; State 0: start state

q0 a _ r q1
q0 b _ r q4
q0 _ _ * acc

q1 a a r q1
q1 b b r q1
q1 _ _ l q2

q2 a _ l q3
q2 b _ l rej
q2 _ _ * rej

q3 a a l q3
q3 b b l q3
q3 _ _ r q0

q4 a a r q4
q4 b b r q4
q4 _ _ l q5

q5 a _ l rej
q5 b _ l q6
q5 _ _ * rej

q6 a a l q6
q6 b b l q6
q6 _ _ r q0
