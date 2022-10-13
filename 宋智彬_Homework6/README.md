## Summary

* [5 points] 提交格式正确，包含所有需要的文件；代码可以在虚拟机下正确 编译运行。 

> 完成。

* [20 points] 包围盒求交：正确实现光线与包围盒求交函数。 

> 完成。

* [15 points] BVH 查找：正确实现 BVH 加速的光线与场景求交。 

> 完成。

* [加分项 20 points] SAH 查找：自学 SAH(Surface Area Heuristic) , 正 确实现 SAH 加速，并且提交结果图片，并在 README.md 中说明 SVH 的实现 方法，并对比 BVH、SVH 的时间开销。(可参考 http://15462.courses.cs .cmu.edu/fall2015/lecture/acceleration/slide_024，也可以查找其他资 料)

> 未完成。

## Result

![binary.png](./images/binary.png)

## Critical Code

```cpp
inline bool Bounds3::IntersectP(const Ray& ray, const Vector3f& invDir,
                                const std::array<int, 3>& dirIsNeg) const
{
    // invDir: ray direction(x,y,z), invDir=(1.0/x,1.0/y,1.0/z), use this because Multiply is faster that Division
    // dirIsNeg: ray direction(x,y,z), dirIsNeg=[int(x>0),int(y>0),int(z>0)], use this to simplify your logic
    // TODO test if ray bound intersects
    float tMinX = (pMin.x - ray.origin.x) * invDir.x;
    float tMaxX = (pMax.x - ray.origin.x) * invDir.x;
    if (dirIsNeg[0])                                   // 如果光线方向为负，值为1；否则为0
        std::swap(tMinX, tMaxX);                       // 如果方向为负，需要交换tMin与tMax

    float tMinY = (pMin.y - ray.origin.y) * invDir.y;
    float tMaxY = (pMax.y - ray.origin.y) * invDir.y;
    if (dirIsNeg[1])
        std::swap(tMinY, tMaxY);

    float tMinZ = (pMin.z - ray.origin.z) * invDir.z;
    float tMaxZ = (pMax.z - ray.origin.z) * invDir.z;
    if (dirIsNeg[2])
        std::swap(tMinZ, tMaxZ);

    float tEnter = std::max({tMinX, tMinY, tMinZ});
    float tExit  = std::min({tMaxX, tMaxY, tMaxZ});

    if (tEnter < tExit && tExit >= 0)
        return true;
    return false;
}
```

```cpp
Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    // TODO Traverse the BVH to find intersection
    std::array<int, 3> dirIsNeg({ray.direction.x < 0, ray.direction.y < 0, ray.direction.z < 0});

    if (!node->bounds.IntersectP(ray, ray.direction_inv, dirIsNeg))
        return Intersection();
    
    if (!node->left && !node->right)
        return node->object->getIntersection(ray);  // 见BVH构建代码知，node->object只存放了一个object；若存放多个，需要遍历求交；
    
    Intersection l = getIntersection(node->left, ray);
    Intersection r = getIntersection(node->right, ray);
    return l.distance < r.distance ? l : r;
}
```

