#include <AppBox/AppBox.h>
#include <AppBox/ArgsParser.h>
#include <Device/Device.h>
#include <Context/Program.h>
#include <ProgramRef/PixelShaderPS.h>
#include <ProgramRef/VertexShaderVS.h>

int main(int argc, char* argv[])
{
    Settings settings = ParseArgs(argc, argv);
    AppBox app("Triangle", settings);

    Context context(settings, app.GetWindow());
    Device& device(*context.GetDevice());
    AppRect rect = app.GetAppRect();
    ProgramHolder<PixelShaderPS, VertexShaderVS> program(device);

    std::shared_ptr<CommandListBox> upload_command_list = context.CreateCommandList();
    upload_command_list->Open();
    std::vector<uint32_t> ibuf = { 0, 1, 2 };
    std::shared_ptr<Resource> index = device.CreateBuffer(BindFlag::kIndexBuffer | BindFlag::kCopyDest, sizeof(uint32_t) * ibuf.size());
    upload_command_list->UpdateSubresource(index, 0, ibuf.data(), 0, 0);
    std::vector<glm::vec3> pbuf = {
        glm::vec3(-0.5, -0.5, 0.0),
        glm::vec3(0.0,  0.5, 0.0),
        glm::vec3(0.5, -0.5, 0.0)
    };
    std::shared_ptr<Resource> pos = device.CreateBuffer(BindFlag::kVertexBuffer | BindFlag::kCopyDest, sizeof(glm::vec3) * pbuf.size());
    upload_command_list->UpdateSubresource(pos, 0, pbuf.data(), 0, 0);
    upload_command_list->Close();
    context.ExecuteCommandLists({ upload_command_list });

    program.ps.cbuffer.Settings.color = glm::vec4(1, 0, 0, 1);

    std::vector<std::shared_ptr<CommandListBox>> command_lists;
    for (uint32_t i = 0; i < Context::FrameCount; ++i)
    {
        decltype(auto) command_list = context.CreateCommandList();
        command_list->Open();
        command_list->UseProgram(program);
        command_list->Attach(program.ps.cbv.Settings, program.ps.cbuffer.Settings);
        command_list->SetViewport(rect.width, rect.height);
        command_list->Attach(program.ps.om.rtv0, context.GetBackBuffer(i));
        command_list->ClearColor(program.ps.om.rtv0, { 0.0f, 0.2f, 0.4f, 1.0f });
        command_list->IASetIndexBuffer(index, gli::format::FORMAT_R32_UINT_PACK32);
        command_list->IASetVertexBuffer(program.vs.ia.POSITION, pos);
        command_list->DrawIndexed(3, 0, 0);
        command_list->Close();
        command_lists.emplace_back(command_list);
    }

    while (!app.PollEvents())
    {
        context.ExecuteCommandLists({ command_lists[context.GetFrameIndex()] });
        context.Present();
        app.UpdateFps();
    }
    context.WaitIdle();
    return 0;
}
