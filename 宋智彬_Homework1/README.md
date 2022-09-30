[toc]

## Summary

1. [5 分] 正确构建模型矩阵。

> 完成。

2. [5 分] 正确构建透视投影矩阵。 

> 完成。

2. [10 分] 你的代码可以在现有框架下正确运行，并能看到变换后的三角形。 

> 完成。

2. [10 分] 当按 A 键与 D 键时，三角形能正确旋转。或者正确使用命令行得 到旋转结果图像。 

> 完成。

2. [提高项 5 分] 在 main.cpp 中构造一个函数，该函数的作用是得到绕任意 过原点的轴的旋转变换矩阵。

> 部分完成。思路及代码放在了下面，并没有写入到`main.cpp`中，有两个问题，1.不太清楚如何在现有框架之中去验证正确性；2.该函数代码有问题，编译并不能通过。

## Eigen::Matrix4f get_model_matrix(float rotation_angle)

* 要求：逐个元素地构建模型变换矩阵并返回该矩阵。在此函数中，你只需要实现三维中绕 z 轴旋转的变换矩阵，而不用处理平移与缩放。
* 使用到的知识：绕z轴旋转的旋转矩阵（4*4）
* 代码：
```cpp
Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    float theta = rotation_angle / 180 * MY_PI;     // 角度制->弧度制
    float cos = std::cos(theta), sin = std::sin(theta);

    model << cos, -sin, 0, 0,
            sin, cos, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1;

    return model;
}
```

## Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,float zNear, float zFar)
* 要求：使用给定的参数逐个元素地构建透视投影矩阵并返回该矩阵。
* 知识点：投影矩阵 = 正交矩阵 * 透视矩阵，即对于物体，先进行透视变换（squish成一个立方体），再进行正交变换（先平移中心点到坐标系原点，再缩放成$[-1, 1]^3$的正方体，即$M_{projection} = M_{scale} * M_{trans} * M_{perspective}$
* 代码：
```cpp
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();      // projection = perspective + orthographic

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

    Eigen::Matrix4f persp;    // Mpersp->orthor
    persp << zNear, 0, 0, 0,
            0, zNear, 0, 0,
            0, 0, zNear + zFar, -zNear * zFar, 
            0, 0, 1, 0;
    
    float theta = eye_fov / 2 / 180 * MY_PI;    // 角度->弧度，取1/2（类比只看上半边的直角三角形）
    float b = std::tan(theta) * zNear, t = -b;
    float r = t / aspect_ratio, l = -r;
    
    Eigen::Matrix4f ortho = Eigen::Matrix4f::Identity();    // Mortho = Mscale * Mtrans（先平移，再缩放）
    
    Eigen::Matrix4f trans;
    trans << 1, 0, 0, 0,    // [1, 0, 0, A]，其中 A == -(l+r)/2 == 0;
            0, 1, 0, 0,     // [0, 1, 0, B]，其中 B == -(b+t)/2 == 0;
            0, 0, 1, - (zNear + zFar) / 2,
            0, 0, 0, 1;
    
    Eigen::Matrix4f scale;
    scale << 1/r, 0, 0, 0,  // 2/(r-l) == 1/r
            0, 1/t, 0, 0,   // 2/(t-b) == 1/t
            0, 0, 2/(zNear-zFar), 0,
            0, 0, 0, 1;

    projection = scale * trans * persp;
    return projection;
}
```

## [提高项 5 分] 在 main.cpp 中构造一个函数，该函数的作用是得到绕任意过原点的轴的旋转变换矩阵。
* `Eigen::Matrix4f get_rotation(Vector3f axis, float angle)`
* 使用到罗德里格斯旋转公式(Rodrigues' Rotation Formula)
$R(n, \alpha) = cos(\alpha)I + (1-cos(\alpha))nn^T + sin(\alpha)
\underbrace{\begin{pmatrix}
0 & -n_z & n_y \\
n_z & 0 & -n_x \\
-n_y & n_x & 0
\end{pmatrix}}_N$
* 代码：（未经验证，纯表思路）
```cpp
Eigen::Matrix4f get_rotation(Vector3f axis, float angle)
{
    Eigen::Matrix4f rotation = Eigen::Matrix4f::Identity();

    float alpha = angle / 180 * MY_PI;
    Eigen::Matrix3f rodrigues;

    Eigen::Matrix3f N;
    N << 0, -axis.z(), axis.y(),
        axis.z(), 0, -axis.x(),
        -axis.y(), axis.x(), 0;

    rodrigues = std::cos(alpha) * Eigen::Matrix3f::Identity() + 
                (1 - std::cos(alpha)) * axis.cross(axis.transpose()) +
                std::sin(alpha) * N;

    rotation.block<3, 3>(0, 0) = rodrigues;

    return rotation;
}
```