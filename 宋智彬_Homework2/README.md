[toc]

## Summary

* [5 分] 正确地提交所有必须的文件，且代码能够编译运行。 

>完成。

* [20 分] 正确实现三角形栅格化算法。 

>完成。

* [10 分] 正确测试点是否在三角形内。 

>完成。

* [10 分] 正确实现 z-buffer 算法, 将三角形按顺序画在屏幕上。 

>完成。
>
>![无MSAA](images/origin.png)

* [提高项 5 分] 用 super-sampling 处理 Anti-aliasing : 你可能会注意 到，当我们放大图像时，图像边缘会有锯齿感。我们可以用 super-sampling 来解决这个问题，即对每个像素进行 2 * 2 采样，并比较前后的结果 (这里 并不需要考虑像素与像素间的样本复用)。需要注意的点有，对于像素内的每 一个样本都需要维护它自己的深度值，即每一个像素都需要维护一个 sample list。最后，如果你实现正确的话，你得到的三角形不应该有不正常的黑边。

>完成。（使用MSAA变量控制实现。没有黑边，但是在两个图形的交界处会有灰边（两种颜色按比例分配混合得到的颜色）
>
>![MSAA](images/MSAA.png)

## 测试点是否在三角形内

* 要点：选择叉乘进行判断。若使用点成，只能知道前后关系，无法判断左右关系。
* 框架改动：将传入的参数中x、y改为了float类型，便于更精准的判断每一个测试点是否在三角形内。
* 代码：

```cpp
static bool insideTriangle(float x, float y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    float xAB = _v[1].x() - _v[0].x(), yAB = _v[1].y() - _v[0].y();
    float xBC = _v[2].x() - _v[1].x(), yBC = _v[2].y() - _v[1].y();
    float xCA = _v[0].x() - _v[2].x(), yCA = _v[0].y() - _v[2].y();

    float xAP = x - _v[0].x(), yAP = y - _v[0].y();
    float xBP = x - _v[1].x(), yBP = y - _v[1].y();
    float xCP = x - _v[2].x(), yCP = y - _v[2].y();

    float zABAP = xAB * yAP - yAB * xAP, zBCBP = xBC * yBP - yBC * xBP, zCACP = xCA * yCP - yCA * xCP;

    if (zABAP < 0 && zBCBP < 0 && zCACP < 0 || zABAP > 0 && zBCBP > 0 && zCACP > 0)
        return true;
    else
        return false;
}
```

## 实现三角形栅格化算法 + z-buffer算法，按顺序画出三角形

* 要点：
    1. 由pdf文档知，z-buffer算法中z全为正，且z越小越接近屏幕，要绘制z较小的颜色；
    2. 对于原始实现与MSAA实现，使用bool变量`MSAA`进行区别。
    3. 先判断测试点是否有在三角形内，再判断z轴深度是否有更新的必要；顺序反过来亦可。（我的实现是先判断测试点是否在三角形内，因为我觉得通过插值求z的运算量比调用判断测试点是否在三角形内的开销高，但是这里并没有进行基准测试，一切纯主观）
* 代码：

```cpp
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    
    bool MSAA = true;
    
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle
    float left = std::min(v[0].x(), std::min(v[1].x(), v[2].x()));
    float right = std::max(v[0].x(), std::max(v[1].x(), v[2].x()));

    float bottom = std::min(v[0].y(), std::min(v[1].y(), v[2].y()));
    float top = std::max(v[0].y(), std::max(v[1].y(), v[2].y()));

    // If so, use the following code to get the interpolated z value.
    //auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
    //float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
    //float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
    //z_interpolated *= w_reciprocal;

    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
    if (MSAA)
    {
        for (int i = left; i <= right; ++i)
        {
            for (int j = bottom; j <= top; ++j) 
            {
                float x1 = i + 0.25f, y1 = j + 0.25f;
                float x2 = i + 0.75f, y2 = j + 0.25f;
                float x3 = i + 0.25f, y3 = j + 0.75f;
                float x4 = i + 0.75f, y4 = j + 0.75f;
                int cnt = int(insideTriangle(x1, y1, t.v)) +
                         int(insideTriangle(x2, y2, t.v)) + 
                         int(insideTriangle(x3, y3, t.v)) +
                         int(insideTriangle(x4, y4, t.v));
                
                if (cnt > 0)
                {
                    auto[alpha, beta, gamma] = computeBarycentric2D(i + 0.5f, j + 0.5f, t.v);
                    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                    z_interpolated *= w_reciprocal;

                    if (z_interpolated < depth_buf[get_index(i, j)])    // 越小越接近相近，此时更新：设置像素颜色并更新深度缓冲区
                    {
                        Eigen::Vector3f color = t.getColor() * cnt / 4 + frame_buf[get_index(i, j)] * (4-cnt) / 4;
                        set_pixel(Eigen::Vector3f(i, j, z_interpolated), color);
                        depth_buf[get_index(i, j)] = z_interpolated;
                    }
                }
            }
        }
    }
    else
    {
        for (int i = left; i <= right; ++i)
        {
            for (int j = bottom; j <= top; ++j) 
            {
                float x = i + 0.5f, y = j + 0.5f;
                if (insideTriangle(x, y, t.v))
                {
                    auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
                    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                    z_interpolated *= w_reciprocal;

                    if (z_interpolated < depth_buf[get_index(i, j)])    // 越小越接近相近，此时更新：设置像素颜色并更新深度缓冲区
                    {
                        set_pixel(Eigen::Vector3f(i, j, z_interpolated), t.getColor());
                        depth_buf[get_index(i, j)] = z_interpolated;
                    }
                }
            }
        }
    }
}
```

