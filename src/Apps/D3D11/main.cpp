#include "DX11Scene.h"
#include <AppBox/AppBox.h>

int main(void)
{
    return AppBox(DX11Scene::Create, ApiType::kDX, "[DX11] testApp", 1280, 720).Run();
}