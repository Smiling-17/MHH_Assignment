#include "utils.h"

int main() {
    Model net;
    net.places = {"p1", "p2"};
    net.transitions = {"t1"};
    net.Pre = {{1}, {0}};
    net.Post = {{0}, {1}};
    net.M0 = {1, 0};

    cout << "Initial marking: " << toString(net.M0) << endl;
    if (isEnabled(net, net.M0, 0)) {
        auto M1 = fire(net, net.M0, 0);
        cout << "After firing t1: " << toString(M1) << endl;
    }
    return 0;
}
