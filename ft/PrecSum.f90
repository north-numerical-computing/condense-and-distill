      module PrecSum_m
        use, intrinsic :: iso_c_binding

        implicit none
        double precision, parameter :: u = 2.d0**(-53)
        contains

      subroutine prec_sum(n, p, k, s) bind(c, name="ft_prec_sum")
        integer, intent(in) :: n, k
        double precision, intent(in) :: p(n)
        double precision, intent(out) :: s
!       local
        integer :: L, i, j
        double precision, allocatable :: tau(:), sigma(:)
        double precision :: mu, Ms, phi, t, q, M, e, pi, x, Pbig

        mu = sum(abs(p))/(1.d0-2.d0*dble(n)*u)
        Ms = NextPowerTwo(dble(n+2))
        M = log2(Ms)
        L = ceiling( ( dble(k)*log2(u) - 2.d0 ) / ( log2(u) + M ) ) - 1
        write(*,*) 'L=',L
        allocate(sigma(L+1), tau(L))
        sigma = 0.d0
        tau = 0.d0
        sigma(1) = NextPowerTwo(mu)

        phi = Ms*u
        do i=1,L
          sigma(i+1) = phi*sigma(i)
        enddo

        tau = 0.d0
        Pbig = 0.d0
        do j=1,n
          pi = p(j)
          do i=1,L
            q = (sigma(i) + pi) - sigma(i)
            pi = pi - q
            tau(i) = tau(i) + q
          enddo
          Pbig = Pbig + pi
        enddo

        pi = tau(1)
        e = 0.d0
        do i=2,L
          x = pi
          pi = x + tau(i)
          t = tau(i) - (pi - x)
          e = e + t
        enddo
        s = pi + (e + Pbig)
      end subroutine prec_sum

      function log2(x) result(y)
        double precision :: x,y
        y = log(x)/log(2.d0)
      end function log2

      function NextPowerTwo(p) result(L)
        double precision :: p, L
        ! local
        double precision :: q
        q = p/u;
        L = abs( ( q-p ) - q );
      end function NextPowerTwo
      end module
