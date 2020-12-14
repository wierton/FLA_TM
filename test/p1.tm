; This example program checks if the input string is a binary palindrome.
; Input: a string of 0's and 1's, e.g. '1001001'

; the finite set of states
#Q = {q1,q2,q3,q4,q5,acc,rej}

; the finite set of input symbols
#S = {0}

; the complete set of tape symbols
#G = {0,1,_}

; the start state
#q0 = q1

; the blank symbol
#B = _

; the set of final states
#F = {acc}

; the number of tapes
#N = 1

; the transition functions

; State 0: start state

q1 _ _ r rej
q1 0 _ r q2
q1 1 1 r rej

q2 _ _ r acc
q2 0 1 r q3
q2 1 1 r q2

q3 _ _ l q5
q3 0 0 r q4
q3 1 1 r q3

q4 _ _ r rej
q4 0 1 r q3
q4 1 1 r q4

q5 _ _ r q2
q5 0 0 l q5
q5 1 1 l q5
