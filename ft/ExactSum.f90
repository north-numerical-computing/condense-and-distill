      module exactsum_m
        use, intrinsic :: iso_c_binding

        implicit none
        integer, parameter :: ebits = 1024
        type accumulator
          double precision :: ON(2*ebits) = 0.d0
          double precision :: OP(2*ebits) = 0.d0
          double precision :: EN(2*ebits) = 0.d0
          double precision :: EP(2*ebits) = 0.d0
        end type
        type merged_acc
          double precision, pointer :: val(:)
        end type
        contains

      subroutine exact_sum(n, x, sacc, nacc) bind(c, name="ft_exact_sum")
        integer, intent(in) :: n
        double precision, intent(in) :: x(n)
        double precision, pointer, intent(out) :: sacc(:)
        integer, intent(out) :: nacc
!       local
        type(accumulator) :: acc
        integer :: i, e, lbp
        logical :: positive

        do i=1,n
          if (x(i).eq.0.d0) cycle
          e = exponent(x(i)) + ebits
          positive = (x(i).gt.0)
          call last_bit_parity(x(i), lbp)
          call add_to_accumulator(x(i), acc, e, positive, lbp)
        enddo
        call merge_accumulators(acc, sacc, nacc)

      end subroutine exact_sum

      recursive subroutine add_to_accumulator(x, acc, e, positive, lbp)
        double precision, intent(in) :: x
        integer, intent(in) :: e, lbp
        logical, intent(in) :: positive
        type(accumulator), target, intent(inout) :: acc
!       local
        double precision, pointer :: acc_ptr(:)
        double precision :: x_new
        integer :: lbp_new
        
        if (positive) then
          if (lbp.eq.1) then
            acc_ptr => acc%OP
          else
            acc_ptr => acc%EP
          endif
        else
          if (lbp.eq.1) then
            acc_ptr => acc%ON
          else
            acc_ptr => acc%EN
          endif
        endif

        if (acc_ptr(e).eq.0.d0) then
          acc_ptr(e) = x
        else
          x_new = x + acc_ptr(e)
          !errloc = abs(dble(real(x_new,16)-real(x,16)-real(acc_ptr(e),16)))
          acc_ptr(e) = 0.d0
          call last_bit_parity(x_new, lbp_new)
          call add_to_accumulator(x_new, acc, e+1, positive, lbp_new)
        endif

      end subroutine add_to_accumulator

      subroutine merge_accumulators(acc, sacc, nacc)
        type(accumulator), target, intent(in) :: acc
        double precision, pointer, intent(out) :: sacc(:)
        integer, intent(out) :: nacc
        ! local
        integer :: nacc1, nacc2, nacc3, nacc4
        
        call get_accumulator_size(acc%OP, nacc1)
        call get_accumulator_size(acc%ON, nacc2)
        call get_accumulator_size(acc%EP, nacc3)
        call get_accumulator_size(acc%EN, nacc4)
        nacc = nacc1+nacc2+nacc3+nacc4
        allocate(sacc(nacc))
        call empty_accumulator(acc%OP, sacc, 1)
        call empty_accumulator(acc%ON, sacc, nacc1+1)
        call empty_accumulator(acc%EP, sacc, nacc1+nacc2+1)
        call empty_accumulator(acc%EN, sacc, nacc1+nacc2+nacc3+1)
      end subroutine merge_accumulators

      subroutine get_accumulator_size(acc, nacc)
        double precision, intent(in) :: acc(2*ebits)
        integer, intent(out) :: nacc
        ! local
        integer :: e

        nacc = 0
        do e=1,2*ebits
          if (acc(e).ne.0d0) then
            nacc = nacc + 1
          endif
        enddo
      end subroutine get_accumulator_size

      subroutine empty_accumulator(acc, sacc, ibeg)
        double precision, intent(in) :: acc(2*ebits)
        double precision, intent(inout) :: sacc(:)
        integer, intent(in) :: ibeg
        ! local
        integer :: e, i

        i = ibeg
        do e=1,2*ebits
          if (acc(e).ne.0d0) then
            sacc(i) = acc(e)
            i = i+1
          endif
        enddo
      end subroutine empty_accumulator

      end module

      subroutine last_bit_parity(x, lbp)
        integer(8), intent(in) :: x
        integer, intent(out) :: lbp
        lbp = ibits(x, 0, 1)
      end subroutine last_bit_parity
