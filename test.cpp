#include <iostream>
#include <cstdio>

using std::cout;

class test
{
private:
    int num;

public:
    test(/* args */);
    ~test();

    int increase();
    int getNum();
};

test::test(/* args */)
{
    num = 0;
}

test::~test()
{
}

int test::increase(/* args */)
{
    static int invokeTimes = 0;

    num++;
    invokeTimes++;

    return invokeTimes;
}

int test::getNum(/* args */)
{
    return num;
}

enum UWBFrame_Type : uint8_t
{
    No_Frame = 0x00,
    //UWB_Anchor_Frame0 = 0x01,
    //UWB_Tag_Frame0 = 0x02,
    UWB_Node_Frame0 = 0x04,
    //UWB_Node_Frame1 = 0x08,
    UWB_Node_Frame2 = 0x10,
};

int main(void)
{
    test a, b;
    int invoketimes;
    a.increase();
    b.increase();
    invoketimes = b.increase();
    cout << "a = " << a.getNum() << ", b = " << b.getNum() << ", function increase() is invoked for " << invoketimes << " times.\n";

    uint8_t testbit;
    testbit = UWB_Node_Frame0 | UWB_Node_Frame2;
    printf("0x%02X\n", testbit);
    testbit &= (~UWB_Node_Frame0);
    printf("0x%02X\n", testbit);

    return 0;
}