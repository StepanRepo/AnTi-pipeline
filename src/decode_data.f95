subroutine decode_data_f(data, signal, n) BIND(C)
USE, INTRINSIC :: ISO_C_BINDING
implicit none

        integer(kind = C_INT) :: n
        integer(kind = C_INT), dimension(1:n) :: data, pow
        real(kind = C_DOUBLE), dimension (1:n) :: signal

        real(8) :: coef =  1.3565771745707199e-14*0.2024_8/1.2288_8

        print*, data(1)

        pow = ishft(iand(data, z'7F000000'), 24) - 64
        data = iand(data, z'FFFFFF')
        print*,  pow(1)

        signal = real(data, 8) * coef

end subroutine
