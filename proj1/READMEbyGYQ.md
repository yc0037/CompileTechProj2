# README

## 2020-05-15 Update

修复了函数体中数组打印格式的错误。

增加了输出带括号表达式的功能。为`Binary::make()`函数增加了一个参数`print_bracket`，默认为`false`。用法参见`gemm.cc`。

修改的文件包括：

- `IRPrinter.cc`，修改了对`Var`的``visit`函数
- `IR.h`，修改了`Binary`类的`make`函数
  - 在`gemm.cc`里增加了对输出带括号表达式功能的示例
- `IRMutator.cc`，修改了其中对`Bianry::make`的调用

## 2020-05-06 Update

增加了json解析的功能。使用了[JSON for Modern C++](https://github.com/nlohmann/json)库。

在`solution.cc`中，已经将json文件中的各个属性读到对应名字的变量中，可以像使用STL类的对象一样使用。如：

```c++
// solution.cc
		std::string sname;
    ir["name"].get_to(sname);
    std::vector<std::string> sins;
    ir["ins"].get_to(sins);
    std::cout << "name: " << sname << std::endl;
    std::cout << "ins: ";
    for(size_t i = 0; i < sins.size(); ++i) {
      std::cout << sins[i] << " ";
    }
```

## 2020-05-05

初步完成了对IRPrinter的修改，用`./test`下的`conv2d.cc`和`gemm.cc`进行测试，能够顺利运行。

修改的文件包括：

- `IRPrinter.cc`，修改了各个`visit`函数
- `IR.h`，修改了`Dom`类的`make`函数，在调用时需要传入范围对应的符号
  - 在`gemm.cc`和`conv2d.cc`里对`Dom::make`的调用进行了相应的修改
- `IRMutator.cc`，修改了其中对`Dom::make`的调用

`b.sh`是一个脚本文件，可以自动执行使用`cmake`进行编译的命令。使用方法为

```shell
sh ./b.sh
```

