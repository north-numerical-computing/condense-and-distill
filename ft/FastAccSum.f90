      module FastAccSum_m
        use, intrinsic :: iso_c_binding

        implicit none
        double precision, parameter :: u = 2.d0**(-53)
        double precision, parameter :: realmin = 2.2251d-308
        contains

      subroutine fast_acc_sum(n, p, s) bind(c, name="ft_fast_acc_sum")
        integer, intent(in) :: n
        double precision, intent(inout) :: p(n)
        double precision, intent(out) :: s
!       local
        double precision :: tau1, tau2, tau, Tbig, tp, v, Phi
        double precision :: mu, sigma, sigmap, sigma0, t, q
        double precision :: c1, c2, c3, c4, c5, c6, c7, c8, c9
        integer :: i

        s = 0.d0
        c1 = 1-n*u;
        c2 = 1-(3*n+1)*u;
        c3 = 2*u;
        c4 = 1-u;
        c5 = 2*n*(n+2)*u;
        c6 = 1-5*u;
        c7 = (1.5+4*u)*(n*u);
        c8 = 2*n*u;
        c9 = realmin/u;

        Tbig = sum(abs(p))/c1
        tp = 0.d0
        do while (.true.)
          sigma0 = (2*Tbig)
          sigma = sigma0
          do i=1,n
            if (p(i).eq.0.d0) cycle
            sigmap = sigma + p(i)
            q = sigmap - sigma
            p(i) = p(i) - q
            sigma = sigmap
          enddo
          tau = sigmap-sigma0
          t = tp
          tp = t + tau
          if (tp.eq.0.d0) then
            call Fast_Acc_Sum(n, p, s)
            return
          endif
          q = sigma0/c3
          v = abs(q/c4 - q)
          Phi = (c5*v) / c6
          Tbig = min( c7*sigma0 , c8*v )
          if ((abs(tp) .ge. Phi ) .or. (4*Tbig .le. c9)) then
            tau2 = (t-tp) + tau
            s = tp + (tau2 + sum(p))
            return
          endif
        enddo
      end subroutine fast_acc_sum

      end module
