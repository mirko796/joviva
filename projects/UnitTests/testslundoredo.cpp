#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#include "utest.h"
#pragma GCC diagnostic pop
#include "jiundoredo.h"

UTEST(UndoRedo,basics)
{
    const int m_initialHead = 123456;
    JIUndoRedo<int> ur(m_initialHead);
    // verify initial state
    ASSERT_EQ(1,ur.stackSize());
    ASSERT_EQ(0,ur.stackPosition());
    ASSERT_FALSE(ur.canRedo());
    ASSERT_FALSE(ur.canUndo());
    ASSERT_EQ(m_initialHead,ur.head());

    // verify with two elements
    ur.push(2);
    ASSERT_EQ(2,ur.stackSize());
    ASSERT_EQ(1,ur.stackPosition());
    ASSERT_FALSE(ur.canRedo());
    ASSERT_TRUE(ur.canUndo());
    ASSERT_EQ(2,ur.head());

    // verify after undo to initial state
    ASSERT_TRUE(ur.undo());
    ASSERT_EQ(2,ur.stackSize());
    ASSERT_EQ(0,ur.stackPosition());
    ASSERT_TRUE(ur.canRedo());
    ASSERT_FALSE(ur.canUndo());
    ASSERT_EQ(m_initialHead,ur.head());
    ASSERT_FALSE(ur.undo());

    // verify after redo from initial state
    ASSERT_TRUE(ur.redo());
    ASSERT_EQ(2,ur.stackSize());
    ASSERT_EQ(1,ur.stackPosition());
    ASSERT_FALSE(ur.canRedo());
    ASSERT_TRUE(ur.canUndo());
    ASSERT_EQ(2,ur.head());
    ASSERT_FALSE(ur.redo());

    // verify adding a new element after undo
    ASSERT_TRUE(ur.undo());
    ur.push(10);
    ASSERT_EQ(2,ur.stackSize());
    ASSERT_EQ(1,ur.stackPosition());
    ASSERT_FALSE(ur.canRedo());
    ASSERT_TRUE(ur.canUndo());
    ASSERT_EQ(10,ur.head());

    // hammer everything with more elements
    const int initialSize = ur.stackSize();
    const int initialPos = ur.stackPosition();
    const int iterations = 10;
    for(int i=0;i<iterations;++i) {
        ur.push(ur.head()+1);
        ASSERT_EQ(i+initialPos+1, ur.stackPosition());
        ASSERT_EQ(i+initialSize+1, ur.stackSize());
    }
    for(int i=0;i<iterations;++i) {
        const int prevHead = ur.head();
        ASSERT_TRUE(ur.undo());
        ASSERT_EQ(prevHead-1,ur.head());
        ASSERT_EQ(initialSize+iterations,ur.stackSize());
        ASSERT_EQ(initialPos+iterations-i-1,ur.stackPosition());
    }
    ur.push(123456);
    ASSERT_EQ(123456,ur.head());
    ASSERT_FALSE(ur.canRedo());
    ASSERT_TRUE(ur.canUndo());
    ASSERT_EQ(initialSize+1, ur.stackSize());
    ASSERT_EQ(initialPos+1, ur.stackPosition());

    // verify that reset restores initial state
    ur.reset(123);
    ASSERT_EQ(1,ur.stackSize());
    ASSERT_EQ(0,ur.stackPosition());
    ASSERT_FALSE(ur.canRedo());
    ASSERT_FALSE(ur.canUndo());
    ASSERT_EQ(123,ur.head());
}

