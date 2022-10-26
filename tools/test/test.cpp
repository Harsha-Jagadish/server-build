#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include "InterpretJson.h"
#include "game.h"

using namespace ns;
using namespace std;
int main(){
    Game g;
        
        string path = PATH_TO_JSON;
        
        InterpretJson j(path);
        
        cout << "Full json object: " << endl;
        cout << j.getData() << endl << endl;
        
        InterpretJson data(j);
        data.interpret(g);
        cout << "Data from game object: name, audience, player count min and max: " << endl;
        g.printInfo();

    return 0;
}