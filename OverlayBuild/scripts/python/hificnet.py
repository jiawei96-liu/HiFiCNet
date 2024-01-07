import cmd
import overlay
import underlay

class HificnetCmd(cmd.Cmd):
    prompt = 'HIFICNET> '  # 设置命令行提示符

    def do_overlay(self, arg):
        overlay.OverlayCmd().cmdloop()

    def do_underlay(self, arg):
        underlay.UnderlayCmd().cmdloop()

    def do_exit(self, arg):
        """退出命令行解释器"""
        return True
    do_q = do_exit

if __name__ == '__main__':
    # 创建并运行命令行解释器
    HificnetCmd().cmdloop()