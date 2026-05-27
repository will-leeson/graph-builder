(set-logic QF_BV)

(declare-const c1 (_ BitVec 32))

(declare-const c2 (_ BitVec 32))

(declare-const c3 (_ BitVec 32))

(define-fun $e1 () (_ BitVec 32) 

 (bvadd #b00000000000000000000000000000001 c1))

(define-fun $e2 () (_ BitVec 32) 

 (bvadd #b00000000000000000000000000000001 c1))


(assert (= c2 c3))

(assert (= c3 $e1))

(assert (= c3 $e2))

(assert (= c2 $e1))

(check-sat)

(exit)