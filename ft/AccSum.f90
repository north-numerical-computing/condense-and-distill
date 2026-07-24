      module AccSum_m
        use, intrinsic :: iso_c_binding

        implicit none
        double precision, parameter :: u = 2.d0**(-53)
        double precision, parameter :: realmin = 2.2251d-308
        contains

      subroutine acc_sum(n, p, s) bind(c, name="ft_acc_sum")
        integer, intent(in) :: n
        double precision, intent(inout) :: p(n)
        double precision, intent(out) :: s
!       local
        double precision :: tau1, tau2

        call transform(n, p, tau1, tau2)
        s = tau1 + ( tau2 + sum(p) )

      end subroutine acc_sum

      subroutine transform(n, p, tau1, tau2)
        integer, intent(in) :: n
        double precision, intent(inout) :: p(n)
        double precision, intent(out) :: tau1, tau2
!       local
        double precision :: mu, sigma, Ms, phi, Phi_big, t, tau, q(n)

        mu = maxval(abs(p))
        Ms = NextPowerTwo(dble(n+2))
        sigma = Ms*NextPowerTwo(mu)
        phi = Ms*u
        Phi_big = Ms*Ms*u
        t = 0.d0
        do while (.true.)
          q = ( sigma + p ) - sigma
          tau = sum(q)
          p = p - q
          tau1 = t + tau
          if ((abs(tau1) .ge. Phi_big*sigma) .OR. (sigma.le.realmin)) then
            tau2 = tau - ( tau1 - t ); 
            return
          endif
          t = tau1;          
          sigma = phi*sigma; 
        enddo
      end subroutine transform

      function NextPowerTwo(p) result(L)
        double precision :: p, L
        ! local
        double precision :: q
        q = p/u;
        L = abs( ( q-p ) - q );
      end function NextPowerTwo

      end module
