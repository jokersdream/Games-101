//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

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
