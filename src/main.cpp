#include <iostream>
#include <set>
#include <sstream>

#include <cqcppsdk/cqcppsdk.h>

#include <time.h>
#include <ctime>

using namespace cq;
using namespace std;
using Message = cq::message::Message;
using MessageSegment = cq::message::MessageSegment;

CQ_INIT {
    on_enable([] { logging::info("欢迎", "欢迎使用GeekDT-bot"); });

    on_private_message([](const PrivateMessageEvent &event) {
        try {
            const auto msg = Message(event.message); // 从消息事件的消息内容解析 Message 对象
            for (const auto &seg : msg) { // 遍历消息段
                if (seg == MessageSegment::text("/时间")) { // 发现时间请求的消息段
                    time_t now = time(0); //基于当前系统时间 获取时间数据
                    char *now_char = ctime(&now); // now转换成字符串
                    Message msg_reply("现在是：");
                    msg_reply += now_char;
                    send_message(event.target, msg_reply);
                    break;
                }
            }
        } catch (ApiError &err) {
            logging::warning("私聊", "私聊消息复读失败, 错误码: " + to_string(err.code));
        }
    });

    on_message([](const MessageEvent &event) {
        logging::debug("消息", "收到消息: " + event.message + "\n实际类型: " + typeid(event).name());
    });

    on_group_message([](const GroupMessageEvent &event) {
        static const set<int64_t> ENABLED_GROUPS = {123456, 123457};
        if (ENABLED_GROUPS.count(event.group_id) == 0) return; // 不在启用的群中, 忽略

        try {
            send_message(event.target, event.message); // 复读
            auto mem_list = get_group_member_list(event.group_id); // 获取群成员列表
            string msg;
            for (auto i = 0; i < min(10, static_cast<int>(mem_list.size())); i++) {
                msg += "昵称: " + mem_list[i].nickname + "\n"; // 拼接前十个成员的昵称
            }
            send_group_message(event.group_id, msg); // 发送群消息
        } catch (ApiError &) { // 忽略发送失败
        }
        if (event.is_anonymous()) {
            logging::info("群聊", "消息是匿名消息, 匿名昵称: " + event.anonymous.name);
        }
        event.block(); // 阻止当前事件传递到下一个插件
    });

    on_group_upload([](const auto &event) { // 可以使用 auto 自动推断类型
        stringstream ss;
        ss << "您上传了一个文件, 文件名: " << event.file.name << ", 大小(字节): " << event.file.size;
        try {
            send_message(event.target, ss.str());
        } catch (ApiError &) {
        }
    });
}

CQ_MENU(menu_demo_1) {
    logging::info("菜单", "点击菜单1");
}

CQ_MENU(menu_demo_2) {
    try {
        send_private_message(1040898055, "一个来自菜单按钮的测试请求"); //发给开发者的测试消息（这是个测试）
    } catch (ApiError &err) {
        logging::warning("私聊", "[菜单按钮测试] 发送失败，错误码：" + to_string(err.code));
    }
}
