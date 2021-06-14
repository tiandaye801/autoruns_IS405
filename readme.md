

# Windows 安全原理期末大作业

## 一、对Windows系统中各类自启动技术方法的分析和报告

1. Logon——注册表和目录登录时启动项

   ​	这部分的启动项是Windows桌面软件最常见的开机自启动实现方式，我找到的资料中提到的启动项的位置位于以下的注册表目录项中：（win7,64位系统）

   ​	其中根项为HKEY_CURRENT_USER或是HKEY_LOCAL_MACHINE，分别代表仅限于当前用户权限的启动项和整个系统共用的启动项。由于Windows环境下的应用程序普遍对用户权限不加以认真管理，在安装时也经常随意提权，因此大部分的WindowsLogon启动项都位于HKEY_LOCAL_MACHINE下。

   ```c
   ("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\Run");
   ("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\RunOnce");
   ("SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\Run");
   ```

   ​	其中
   "SOFTWARE\Microsoft\Windows\CurrentVersion\Run"和"SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce"这两条是最常见的登录时自启动项位置。Run是大多数程序的自启动键，这个键里记录的应用程序在Windows每次启动时都会自启动；RunOnce则只会启动一次，默认情况下，在命令行运行之前，会删除 RunOnce 密钥的值。应用程序一般也不被允许持续在这个键下插入启动项的值，RunOnce的主要作用是进行各类程序的重启时安装配置。

   ​	Wow6432Node键下的启动项则是64位系统的显式启动项存储位置。按照老师给出的64位系统说明，这种启动可以不通过这个键来隐性地控制程序启动。除此之外我在其他的一些文档里还找到了一些启动项的记录位置，包括
   "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run"、“Software\Microsoft\Windows\CurrentVersion\RunOnceEx”这两个位置。不过我在win7 64位系统映像里并没有找到这两个键，所以也没有进行爬取分析。

 

   ​	还有一类Logon启动位于C:\Users\\$UserName$\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup和C:\ProgramData\Microsoft\Windows\Start Menu\Programs\StartUp这两个目录下，分别对应当前用户和所有用户的目录自启动项存放位置，具体的实现方式是把程序的.lnk格式快捷方式插入到这个目录下。



   ​	总的来说采用这种方式实现开机自启动的应用程序比较少。在隐蔽性方面注册表记录的启动方式要比直接在开始菜单目录上添加项好很多，很多流氓软件就是通过写入注册表的方式来实现自启动的，可能会让用户找不到这些自启动项的具体位置。

   ​	以上部分主要参考的文档包括<https://docs.microsoft.com/en-us/sysinternals/downloads/autoruns>、[Run and RunOnce Registry Keys - Win32 apps | Microsoft Docs](https://docs.microsoft.com/en-us/windows/win32/setupapi/run-and-runonce-registry-keys)、[Boot or Logon Autostart Execution: Registry Run Keys / Startup Folder, Sub-technique T1547.001 - Enterprise | MITRE ATT&CK®](https://attack.mitre.org/techniques/T1547/001/)这三处。

2. Services、Drivers——自启动服务和驱动

   ​	由于驱动可以被认为是一种特殊的服务（事实上驱动和服务所在的注册表键值位置都是一样的），在这里我把这两类自启动项一同进行分析。普通用户可以通过Windows中的服务GUI应用程序或是net start等命令来查询当前活动的服务\驱动。在Windows7系统中服务和驱动的注册表记录位置都位于HKEY_LOCAL_MACHINE\System\\CurrentControlSet\\Services下，对应所有用户的启动项。
   

   
   ​	这里可以看到，服务和登录自启动项的一个比较大的区别是服务（驱动）是作为子键的形式存放在services下的，而不是像登录启动项那样作为键值存放在Run键下的。而且服务的描述参数也比较多，登录启动项在注册表中只记录了描述和可执行文件位置这两个参量。
   
   ​	对于服务和驱动而言首先要处理的就是是否为自启动项，因为服务的活动状态并不一定是开机自启动。
   
   
   

   
   ​	可以得知只有启动类型为自动的服务才会进行开机自启动。在注册表中这个特性是由Start这个值决定的，当Start的值为0-2时服务才能开机自启动，3对应手动启动，4对应禁止启动。因此我的开机自启动检查只爬取了0-2Start值的服务信息。除此之外，type这个值可以用于区分服务和驱动，当Type的值小于4时一般为驱动，否则为系统服务（这个特性检查在我的程序中没有用到）。
   
   ​	ImagePath的值就是任务要求的服务启动路径。但是经过分析，我发现一部分服务在imagepath中记录的路径并不是它们自己的sys文件路径而是svchost引用的间接执行地址。对于这部分服务，它们的地址信息隐藏在它们的parameters子键的ServiceDll值内容中。
   
 
   
   ​	大部分的系统服务dll都位于Windows/System32文件夹下，而系统驱动在drivers文件夹中；其他的第三方驱动或是服务dll基本都位于各自程序的安装目录下。服务和驱动作为自启动项在系统中的隐蔽性非常强，通过注入服务或是驱动的方式可以在win系统中创造后门或是木马软件接口，具有很高的安全风险性。
   
   ​	以上部分主要参考的文档有[SERVICE_REGISTRY_STATE_TYPE - Win32 apps | Microsoft Docs](https://docs.microsoft.com/en-us/windows/win32/api/winsvc/ne-winsvc-service_registry_state_type)、[Services (Services) - Win32 apps | Microsoft Docs](https://docs.microsoft.com/en-us/windows/win32/services/services)、[Finding a Specific Driver - Win32 apps | Microsoft Docs](https://docs.microsoft.com/en-us/windows/win32/multimedia/finding-a-specific-driver)等。
   
   
   
3. Scheduled Tasks——计划任务

   ​	计划任务是一类特殊的自启动项，它的信息内容存储方式和时间调度体制相比前两类自启动项要复杂一些。用户可以通过控制面板中的计划任务应用或是schtasks命令来操作这些任务。在系统中它们的具体执行文件路径信息保存在注册表的HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Schedule\TaskCache项下，其中这个项的Tree子项中可以查询到和系统中计划任务相对应的树状结构以及这些任务所各自独特的GUID，并且在Tasks目录下可以根据这些GUID查到对应的执行目录、启动时间等等具体信息。



   ​	而在系统目录中也能找到存储计划任务执行信息的.xml格式文件，它们位于C:\Windows\System32\Tasks\Microsoft\Windows目录下，在其中可以找到和注册表中树状结构一致的目录结构，在计划目录名所对应的目录中可以找到以xml格式记载的计划任务具体信息。


   ​	计划任务在系统中也有一定的隐蔽性，可以在用户没有察觉的情况下运行某些后台程序，而且计划任务的时间和频率设置方式比起服务或是驱动要更加灵活，可以用于对Windows系统服务器的攻击。

   ​	以上部分主要参考的文档有[WMI Tasks: Scheduled Tasks - Win32 apps | Microsoft Docs](https://docs.microsoft.com/en-us/windows/win32/wmisdk/wmi-tasks--scheduled-tasks)、[Task Scheduler for developers - Win32 apps | Microsoft Docs](https://docs.microsoft.com/en-us/windows/win32/taskschd/task-scheduler-start-page)等。
   
   

## 二、关于这些自启动方法所撰写的自启动项查看软件的技术报告

​	根据以上对Logon、Services、Drivers和Scheduled Tasks这四类自启动项的分析，我编写了自己的Windows自启动项查看软件，开发语言为C++，开发环境为Visual Studio 2019，运行环境为Win7 64位操作系统。

​	该软件是一个控制台应用程序，调用Win32 API，通过用户输入的指令来控制输出哪一种自启动项，输入“s”输出服务和驱动，输入“l”输出登录自启动项，输入“t”输出计划程序启动项，输入“q”时程序退出。以下是程序运行时的一些demo图片：


​	其中程序具体实现的方式是调用windows API中关于这些注册表项查询的函数实现的，比如RegOpenKeyEx、RegQueryInfoKey等。在调用API查询的这一部分我参照了Microsoft官方文档关于注册表操作和遍历输出系统键值的官方代码，比如[Displaying Task Names and States (C++) - Win32 apps | Microsoft Docs](https://docs.microsoft.com/en-us/windows/win32/taskschd/displaying-task-names-and-state--c---)（计划任务）、[Enumerating Services - Win32 apps | Microsoft Docs](https://docs.microsoft.com/en-us/windows/win32/wpd_sdk/enumerating-services)（服务）、[Enumerating Registry Subkeys - Win32 apps | Microsoft Docs](https://docs.microsoft.com/en-us/windows/win32/sysinfo/enumerating-registry-subkeys)（遍历子键示例code）等，并对这些代码的输出结果参照任务要求进行了标准化。除此之外，因为注册表中的变量种类多为微软自定义扩展类（如DWORD等），我也查阅了[Registry Value Types - Win32 apps | Microsoft Docs](https://docs.microsoft.com/en-us/windows/win32/sysinfo/registry-value-types)、[VALENTA (winreg.h) - Win32 apps | Microsoft Docs](https://docs.microsoft.com/en-us/windows/win32/api/winreg/ns-winreg-valenta)等说明文档来对不同的数据变量类型进行操作并统一作为string类型输出到控制台。



## 三、总结和收获

​	本次Windows安全原理的大作业对我来说可谓是十分艰难，在本次课程大作业前我已有将近一整个学期的时间没有写过完整的C/C++应用程序代码了，基本都是在使用python进行一些基础调包级别的代码coding，用的都是别人封装好的工具。有些同学说这个作业用C#写会比较容易，苦于自己并没有什么C#的经验，无奈作罢。好在Google和Github上有相当丰富的参考资料可以查阅，特别是微软的英文官方文档资料库[Developer tools, technical documentation and coding examples | Microsoft Docs](https://docs.microsoft.com/en-us/)，有大量关于注册表操作和win32 API调用的开发实例，甚至还有可以调用的封装好的模块化检索注册表或是启动项的C++代码实例，给了此时对C开发已经相当生疏的我极大的帮助。

​	这次大作业我一开始想开发一个GUI应用程序，但是后来限于对C环境下的Qt操作实在没什么经验，以及混乱的环境配置问题（为这次大作业特意安装了vs，vscode的复杂c++环境实在是难以处理），做了一个简单的输出控制台程序。不过从这次大作业的开发过程中我还是学到了很多，包括注册表的C++基本操作，字符串类型的转换和输入输出，以及查阅外文官方文档的能力，都让我感到受益匪浅，希望今后能够做的更好。

   
