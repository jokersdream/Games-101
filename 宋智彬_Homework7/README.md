
I've finished task ...

## Summary

* [5 points] 提交格式正确，包含所有需要的文件；代码可以在虚拟机下正确 编译运行。 

> 完成。

* [45 points] Path Tracing：正确实现 Path Tracing 算法，并提交分辨率 不小于 512*512，采样数不小于 8 的渲染结果图片。 

> 完成

* [加分项 10 points] 多线程：将多线程应用在 Ray Generation 上，注意 实现时可能涉及的冲突。 

> 完成。

* [加分项 10 points] Microfacet：正确实现 Microfacet 材质，并提交可 体现 Microfacet 性质的渲染结果。

> 未完成。

## Result

* binary_ssp16.png

![binary_ssp16.png](./images/binary_ssp16.png)

* binary_ssp64.png

![binary_ssp64.png](./images/binary_ssp64.png)

## Code

```cpp
// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    Intersection inter_i = intersect(ray);

    if (!inter_i.happened) // 光线与场景没有交点，返回背景色
        return {};
    
    if (inter_i.obj->hasEmit()) // 交点是光源：1.直接光，返回光源的颜色；2.间接光，返回{0}（黑色）
        return depth == 0 ? inter_i.m->getEmission() : Vector3f(0);

    // 交点是物体，颜色来自于 光源的直接照射 和 物体的反射/折射
    Vector3f L_dir(0), L_indir(0);

    Intersection interLight;
    float pdf_light = 0.f;
    sampleLight(interLight, pdf_light);  // 理解代码可知，interLight为从光源到球面的一个半径长的向量

    auto w_s = (interLight.coords - inter_i.coords).normalized();

    Ray r_s(inter_i.coords, w_s);
    Intersection inter_s = intersect(r_s);
    if (inter_s.happened && (inter_s.coords-interLight.coords).norm() < EPSILON)  // 反射光线直达光源，中间没有阻拦
    {
        auto& L_i = interLight.emit;
        auto f_r = inter_i.m->eval(-ray.direction, w_s, inter_i.normal);
        auto cos_theta = dotProduct(w_s, inter_i.normal);
        auto cos_theta_prime = dotProduct(-w_s, interLight.normal);
        auto distance2 = dotProduct(interLight.coords - inter_i.coords, interLight.coords - inter_i.coords);

        L_dir = L_i * f_r * cos_theta * cos_theta_prime / distance2 / pdf_light;
    }
        
    if (get_random_float() > RussianRoulette)
        return L_dir;
    
    auto w_o = inter_i.m->sample(-ray.direction, inter_i.normal).normalized();
    Ray r_o(inter_i.coords, w_o);
    Intersection inter_o = intersect(r_o);
    if (inter_o.happened && !inter_i.obj->hasEmit())
    {
        auto f_r = inter_i.m->eval(-ray.direction, w_o, inter_i.normal);
        auto cos_theta = dotProduct(w_o, inter_i.normal);
        auto pdf_hemi = inter_i.m->pdf(-ray.direction, w_o, inter_i.normal);

        r_o = Ray(inter_o.coords, w_o);
        L_indir = castRay(r_o, depth + 1) * f_r * cos_theta / pdf_hemi / RussianRoulette;
    }
    
    return L_dir + L_indir;
}
```

## Afterword

对于反射中遇到光源的情况，需要与第一次就遇到光源的情况进行区分。具体到代码中就是`return depth == 0 ? inter_i.m->getEmission() : Vector3f(0);`。如果不这么做，会出现大量的、很明显的噪点，并导致提升ssp数量并不一定能带来画质的提升（可能是降低，因为随着ssp的增加，噪点的影响也更大了）。这个问题修复之前合成的图片如下：（分别对应ssp16和ssp64的情况）

![ssp16_beforeFix](./images/binary_ssp16_beforeFix.png)

![ssp16_beforeFix](./images/binary_ssp16_beforeFix.png)
