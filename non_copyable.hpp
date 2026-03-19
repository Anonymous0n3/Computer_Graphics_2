#pragma once 

class NonCopyable
{
  public: 
    NonCopyable (const NonCopyable &) = delete;              // copy-constructor
    NonCopyable & operator = (const NonCopyable &) = delete; // assign operator

  protected:
    NonCopyable () = default;
    ~NonCopyable () = default; // Protected non-virtual destructor
};
