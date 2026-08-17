      module DemmelHidaSum_m
        use, intrinsic :: iso_c_binding

        implicit none
        integer, parameter :: ebits = 1024
        type accumulatorDH
          real(kind=16) :: val(2*ebits) = real(0.d0,16)
        end type
        contains

      subroutine Demmel_Hida_Sum(n, x, s) bind(c, name="ft_Demmel_Hida_Sum")
        integer, intent(in) :: n
        double precision, intent(in) :: x(n)
        double precision, intent(out) :: s
!       local
        type(accumulatorDH) :: acc
        integer :: i, e
        real(kind=16) :: sq

        do i=1,n
          if (x(i).eq.0.d0) cycle
          e = exponent(x(i)) + ebits
          acc%val(e) = acc%val(e) + real(x(i),16)
        enddo
        sq = real(0.d0, 16)
        do e = 2*ebits, 1, -1
          sq = sq + acc%val(e)
        enddo
        s = dble(sq)
      end subroutine Demmel_Hida_Sum

      subroutine Demmel_Hida_Sum_PAR_aux(n, x, acc)
        integer, intent(in) :: n
        double precision, intent(in) :: x(n)
        type(accumulatorDH), intent(out) :: acc
!       local
        integer :: i, e

        do i=1,n
          if (x(i).eq.0.d0) cycle
          e = exponent(x(i)) + ebits
          acc%val(e) = acc%val(e) + real(x(i),16)
        enddo
      end subroutine Demmel_Hida_Sum_PAR_aux

      end module

