#include "utils.h"
#include "commands.h"

int main(int argc, char **argv)
{
    char command;
    AccountType accountType;

    set_server_ip();

    accountType = login();

    command = showMenu(accountType);

    while (command != '0')
    {
        switch (command)
        {
        case '1':
            addExperience();
            break;
        case '2':
            listPeopleByMajor();
            break;
        case '3':
            listPeopleBySkill();
            break;
        case '4':
            listPeopleByGradYear();
            break;
        case '5':
            listAll();
            break;
        case '6':
            retrieveProfileInfo();
            break;
        case '7':
            if(accountType == ADMIN){
                deleteProfile();
                break;
            }
            break;
        case '8':
            if(accountType == ADMIN){
                registerProfile();
                break;
            }
            break;
        default:
            break;
        }

        command = showMenu(accountType);
    }
}
