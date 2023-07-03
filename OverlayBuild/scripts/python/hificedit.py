import tkinter as tk
from tkinter import ttk
import overlay


class HificEdit():
    def __init__(self, master):
        self.overlay = overlay.Overlay()
        master.title('Overlay')
        self.nets_button = ttk.Button(master, text="Networks", command=self.show_nets)
        self.subnets_button = ttk.Button(master, text="Subnets", command=self.show_subnets)
        self.routers_button = ttk.Button(master, text="Routers", command=self.show_routers)
        self.nodes_button = ttk.Button(master, text="Nodes", command=self.show_nodes)
        # self.pingall_button = ttk.Button(master, text="Pingall", command=self.show_pingall)
        # self.nss_button = ttk.Button(master, text="Ns of VMs", command=self.show_nss)
        # self.routernss_button = ttk.Button(master, text="Ns of Routers", command=self.show_routernss)
        
        # self.show_window = tk.Text(master, state='disabled')
        # self.show_window = tk.Text(master)
        self.show_window = tk.Listbox(master, width=100)

        self.nets_button.grid(row=0, column=0, padx=10, pady=10, sticky="N")
        self.subnets_button.grid(row=0, column=1, padx=10, pady=10, sticky="N")
        self.routers_button.grid(row=0, column=2, padx=10, pady=10, sticky="N")
        self.nodes_button.grid(row=0, column=3, padx=10, pady=10, sticky="N")
        self.show_window.grid(row=1, column=0, columnspan=4, sticky="NSEW")

    def show_nets(self):
        self.show_window.delete(0, 'end')
        for i in self.overlay.overlaynets:
            # self.show_window.insert(tk.END, self.overlay.overlaynets)
            self.show_window.insert(tk.END, i)


    def show_subnets(self):
        self.show_window.delete(0, 'end')
        for i in self.overlay.overlaysubnets:
            
            self.show_window.insert(tk.END, i)

    def show_routers(self):
        self.show_window.delete(0, 'end')
        for i in self.overlay.overlayrouters:
            
            self.show_window.insert(tk.END, i)

    def show_nodes(self):
        self.show_window.delete(0, 'end')
        for i in self.overlay.overlaynodes:
            
            self.show_window.insert(tk.END, i)

    # def show_pingall(self):
    #     self.show_window.clear()
    #     self.show_window.insert(Tk.END, self.overlay.overlay)

    # def show_nss(self):
    #     self.show_window.clear()
    #     self.show_window.insert(Tk.END, self.overlay.overl)

    # def show_routernss(self):
    #     self.show_window.clear()
    #     self.show_window.insert(Tk.END, self.overlay.overlaynets)



if __name__ == '__main__':
    root = tk.Tk()
    app = HificEdit(root)
    root.mainloop()


        

