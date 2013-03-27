/*
 *  Calculate the nth item of Fibonacci Series recursively
 */

long long Fibonacci_Solution1(unsigned int n)
{
    int result[2] = {0, 1};
    if (n < 2)
        return result[n];

    return Fibonacci_Solution1(n - 1) + Fibonacci_Solution1(n - 2);
}

/*
 *  Calculate the nth item of Fibonacci Series iteratively
 */

long long Fibonacci_Solution2(unsigned n)
{
    int result[2] = {0, 1};
    if (n < 2)
        return result[n];

    long long  fibNMinusOne = 1;
    long long  fibNMinusTwo = 0;
    long long  fibN = 0;
    for(unsigned int i = 2; i <= n; ++i) {
        fibN = fibNMinusOne + fibNMinusTwo;
        fibNMinusTwo = fibNMinusOne;
        fibNMinusOne = fibN;
    }

    return fibN;
}

/*
 *  {f(n), f(n-1), f(n-1), f(n-2)} ={1, 1, 1,0}n-1
 *  需要求得矩阵{1, 1, 1,0}的n-1次方，因为矩阵{1, 1, 1,0}的n-1次方的结果的第一行第一列就是f(n)
 */  

struct Matrix2By2
{
    Matrix2By2
    (
        long long m00 = 0, 
        long long m01 = 0, 
        long long m10 = 0, 
        long long m11 = 0
    )
    :m_00(m00), m_01(m01), m_10(m10), m_11(m11) 
    {
    }

    long long m_00;
    long long m_01;
    long long m_10;
    long long m_11;
};

///////////////////////////////////////////////////////////////////////
// Multiply two matrices
// Input: matrix1 - the first matrix
//        matrix2 - the second matrix
//Output: the production of two matrices
///////////////////////////////////////////////////////////////////////
Matrix2By2 MatrixMultiply
(
    const Matrix2By2& matrix1, 
    const Matrix2By2& matrix2
)
{
    return Matrix2By2(
            matrix1.m_00 * matrix2.m_00 + matrix1.m_01 * matrix2.m_10,
            matrix1.m_00 * matrix2.m_01 + matrix1.m_01 * matrix2.m_11,
            matrix1.m_10 * matrix2.m_00 + matrix1.m_11 * matrix2.m_10,
            matrix1.m_10 * matrix2.m_01 + matrix1.m_11 * matrix2.m_11);
}

///////////////////////////////////////////////////////////////////////
// The nth power of matrix 
// 1  1
// 1  0
///////////////////////////////////////////////////////////////////////
Matrix2By2 MatrixPower(unsigned int n)
{
    assert(n > 0);

    Matrix2By2 matrix;
    if(n == 1)
        matrix = Matrix2By2(1, 1, 1, 0);
    else if(n % 2 == 0) {
        matrix = MatrixPower(n / 2);
        matrix = MatrixMultiply(matrix, matrix);
    }
    else if(n % 2 == 1) {
        matrix = MatrixPower((n - 1) / 2);
        matrix = MatrixMultiply(matrix, matrix);
        matrix = MatrixMultiply(matrix, Matrix2By2(1, 1, 1, 0));
    }
    return matrix;
}

///////////////////////////////////////////////////////////////////////
// Calculate the nth item of Fibonacci Series using devide and conquer
///////////////////////////////////////////////////////////////////////
long long Fibonacci_Solution3(unsigned int n)
{
    int result[2] = {0, 1};
    if (n < 2)
        return result[n];

    Matrix2By2 PowerNMinus2 = MatrixPower(n - 1);
    return PowerNMinus2.m_00;
}
