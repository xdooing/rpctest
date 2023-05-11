#include<iostream>
#include<string>
#include"test.pb.h"

int main(){

    testspace::GetFriendListResponse response;
    testspace::ResultCode* res = response.mutable_result();
    res->set_errcode(0);
    res->set_errmsg("成功！");

    testspace::User* user1 = response.add_friend_list();
    user1->set_name("zhang san");
    user1->set_sex(testspace::User::MAN);
    user1->set_age(20);

    testspace::User* user2 = response.add_friend_list();
    user2->set_name("li si");
    user2->set_sex(testspace::User::WOMAN);
    user2->set_age(25);

    std::cout << response.friend_list_size() << std::endl;

    return 0;
}

