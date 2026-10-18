import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import ctypes
import os
# Load the compiled C++ banking library
LIB_PATH = os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "libbank.dylib"
)

bank_lib = ctypes.CDLL(LIB_PATH)

# Configure the C++ wrapper functions
bank_lib.bank_create.argtypes = [
    ctypes.c_char_p,
    ctypes.c_void_p,
    ctypes.c_int
]
bank_lib.bank_create.restype = ctypes.c_void_p

bank_lib.bank_destroy.argtypes = [ctypes.c_void_p]
bank_lib.bank_destroy.restype = None
bank_lib.bank_find_client.argtypes = [
    ctypes.c_void_p,
    ctypes.c_char_p,
    ctypes.c_void_p,
    ctypes.c_int,
    ctypes.c_void_p,
    ctypes.c_int,
    ctypes.POINTER(ctypes.c_double),
    ctypes.c_void_p,
    ctypes.c_int
]
bank_lib.bank_find_client.restype = ctypes.c_int

bank_lib.bank_deposit.argtypes = [
    ctypes.c_void_p,
    ctypes.c_char_p,
    ctypes.c_double,
    ctypes.POINTER(ctypes.c_double),
    ctypes.c_void_p,
    ctypes.c_int
]
bank_lib.bank_deposit.restype = ctypes.c_int

bank_lib.bank_withdraw.argtypes = [
    ctypes.c_void_p,
    ctypes.c_char_p,
    ctypes.c_double,
    ctypes.POINTER(ctypes.c_double),
    ctypes.c_void_p,
    ctypes.c_int
]
bank_lib.bank_withdraw.restype = ctypes.c_int
bank_lib.bank_add_client.argtypes = [
    ctypes.c_void_p,
    ctypes.c_char_p,
    ctypes.c_char_p,
    ctypes.c_char_p,
    ctypes.c_double,
    ctypes.c_void_p,
    ctypes.c_int
]
bank_lib.bank_add_client.restype = ctypes.c_int
bank_lib.bank_save.argtypes = [
    ctypes.c_void_p,
    ctypes.c_char_p,
    ctypes.c_void_p,
    ctypes.c_int
]
bank_lib.bank_save.restype = ctypes.c_int
class BankApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Banking Application")
        self.root.geometry("650x550")

        self.bank = None

        title = ttk.Label(
            root,
            text="Banking Management System",
            font=("Arial", 20, "bold")
        )
        title.pack(pady=20)

        load_button = ttk.Button(
            root,
            text="Load Client File",
            command=self.load_file
        )
        load_button.pack(pady=15)

        self.status = ttk.Label(
            root,
            text="No client file loaded"
        )
        self.status.pack(pady=10)



        ttk.Label(root, text="Account Number:").pack(pady=5)

        self.account_entry = ttk.Entry(root)
        self.account_entry.pack(pady=5)

        ttk.Button(
            root,
            text="Find Client",
            command=self.find_client
        ).pack(pady=10)

        self.client_result = ttk.Label(
            root,
            text="Client information will appear here.",
            justify="left"
        )
        self.client_result.pack(pady=10)

        ttk.Label(root, text="Deposit Amount:").pack(pady=5)

        self.deposit_entry = ttk.Entry(root)
        self.deposit_entry.pack(pady=5)

        ttk.Button(
            root,
            text="Deposit Money",
            command=self.deposit_money
        ).pack(pady=10)


        ttk.Label(root, text="Withdrawal Amount:").pack(pady=5)

        self.withdraw_entry = ttk.Entry(root)
        self.withdraw_entry.pack(pady=5)

        ttk.Button(
            root,
            text="Withdraw Money",
            command=self.withdraw_money
        ).pack(pady=10)
       
        ttk.Button(
            root,
            text="Add New Client",
            command=self.open_add_client_window
        ).pack(pady=10)
        ttk.Button(
            root,
            text="Save Client File",
            command=self.save_file
        ).pack(pady=10)       
        self.root.protocol("WM_DELETE_WINDOW", self.close_app)


    def load_file(self):
        filename = filedialog.askopenfilename(
            title="Select Client CSV File",
            filetypes=[("CSV Files", "*.csv")]
        )

        if not filename:
            return

        error = ctypes.create_string_buffer(512)

        new_bank = bank_lib.bank_create(
            filename.encode("utf-8"),
            error,
            len(error)
        )

        if not new_bank:
            messagebox.showerror(
                "Error",
                error.value.decode("utf-8", errors="replace")
            )
            return

        if self.bank is not None:
            bank_lib.bank_destroy(self.bank)

        self.bank = new_bank
        self.status.config(text="Client file loaded successfully!")

        messagebox.showinfo(
            "Success",
            "Bank client file loaded successfully!"
        )

    def close_app(self):
        if self.bank is not None:
            bank_lib.bank_destroy(self.bank)
            self.bank = None

        self.root.destroy()

    def find_client(self):
        if self.bank is None:
            messagebox.showerror("Error", "Load a client file first.")
            return

        account = self.account_entry.get().strip()

        if not account:
            messagebox.showerror("Error", "Enter an account number.")
            return

        name = ctypes.create_string_buffer(256)
        ssn = ctypes.create_string_buffer(256)
        balance = ctypes.c_double()
        error = ctypes.create_string_buffer(512)

        success = bank_lib.bank_find_client(
            self.bank,
            account.encode("utf-8"),
            name, len(name),
            ssn, len(ssn),
            ctypes.byref(balance),
            error, len(error)
        )

        if not success:
            messagebox.showerror(
                "Search Error",
                error.value.decode("utf-8", errors="replace")
            )
            return

        full_ssn = ssn.value.decode("utf-8")
        masked_ssn = "***-**-" + full_ssn[-4:]

        self.client_result.config(
            text=(
                f"Name: {name.value.decode('utf-8')}\n"
                f"SSN: {masked_ssn}\n"
                f"Account: {account}\n"
                f"Balance: ${balance.value:.2f}"
            )
        )

 
    def deposit_money(self):
        if self.bank is None:
            messagebox.showerror("Error", "Load a client file first.")
            return

        account = self.account_entry.get().strip()
        amount_text = self.deposit_entry.get().strip()

        if not account:
            messagebox.showerror("Error", "Enter an account number.")
            return

        try:
            amount = float(amount_text)
        except ValueError:
            messagebox.showerror("Error", "Enter a valid deposit amount.")
            return

        if not (0 < amount < float("inf")):
            messagebox.showerror("Error", "Deposit must be a positive, finite amount.")
            return

        new_balance = ctypes.c_double()
        error = ctypes.create_string_buffer(512)

        success = bank_lib.bank_deposit(
            self.bank,
            account.encode("utf-8"),
            amount,
            ctypes.byref(new_balance),
            error,
            len(error)
        )

        if not success:
            messagebox.showerror(
                "Deposit Error",
                error.value.decode("utf-8", errors="replace")
            )
            return

        messagebox.showinfo(
            "Deposit Successful",
            f"Deposited ${amount:.2f}\n"
            f"New Balance: ${new_balance.value:.2f}"
        )

        self.find_client()
        self.deposit_entry.delete(0, tk.END)

    def withdraw_money(self):
        if self.bank is None:
            messagebox.showerror("Error", "Load a client file first.")
            return

        account = self.account_entry.get().strip()
        amount_text = self.withdraw_entry.get().strip()

        if not account:
            messagebox.showerror("Error", "Enter an account number.")
            return

        try:
            amount = float(amount_text)
        except ValueError:
            messagebox.showerror("Error", "Enter a valid withdrawal amount.")
            return

        if not (0 < amount < float("inf")):
            messagebox.showerror(
                "Error",
                "Withdrawal must be a positive, finite amount."
            )
            return

        new_balance = ctypes.c_double()
        error = ctypes.create_string_buffer(512)

        success = bank_lib.bank_withdraw(
            self.bank,
            account.encode("utf-8"),
            amount,
            ctypes.byref(new_balance),
            error,
            len(error)
        )

        if not success:
            messagebox.showerror(
                "Withdrawal Error",
                error.value.decode("utf-8", errors="replace")
            )
            return

        messagebox.showinfo(
            "Withdrawal Successful",
            f"Withdrew ${amount:.2f}\n"
            f"New Balance: ${new_balance.value:.2f}"
        )

        self.find_client()
        self.withdraw_entry.delete(0, tk.END)

    def open_add_client_window(self):
        if self.bank is None:
            messagebox.showerror("Error", "Load a client file first.")
            return

        window = tk.Toplevel(self.root)
        window.title("Add New Client")
        window.geometry("400x400")

        ttk.Label(window, text="Client Name:").pack(pady=5)
        name_entry = ttk.Entry(window)
        name_entry.pack(pady=5)

        ttk.Label(window, text="SSN:").pack(pady=5)
        ssn_entry = ttk.Entry(window)
        ssn_entry.pack(pady=5)

        ttk.Label(window, text="Account Number:").pack(pady=5)
        account_entry = ttk.Entry(window)
        account_entry.pack(pady=5)

        ttk.Label(window, text="Starting Balance:").pack(pady=5)
        balance_entry = ttk.Entry(window)
        balance_entry.pack(pady=5)

        def submit_client():
            name = name_entry.get().strip()
            ssn = ssn_entry.get().strip()
            account = account_entry.get().strip()
            balance_text = balance_entry.get().strip()

            if not name or not ssn or not account:
                messagebox.showerror("Error", "Complete all client fields.")
                return

            try:
                balance = float(balance_text)
            except ValueError:
                messagebox.showerror("Error", "Enter a valid starting balance.")
                return

            if not (0 <= balance < float("inf")):
                messagebox.showerror("Error", "Balance must be nonnegative and finite.")
                return

            error = ctypes.create_string_buffer(512)

            success = bank_lib.bank_add_client(
                self.bank,
                name.encode("utf-8"),
                ssn.encode("utf-8"),
                account.encode("utf-8"),
                balance,
                error,
                len(error)
            )

            if not success:
                messagebox.showerror(
                    "Add Client Error",
                    error.value.decode("utf-8", errors="replace")
                )
                return

            messagebox.showinfo("Success", "New client added successfully!")
            window.destroy()

            self.account_entry.delete(0, tk.END)
            self.account_entry.insert(0, account)
            self.find_client()

        ttk.Button(
            window,
            text="Save New Client",
            command=submit_client
        ).pack(pady=15)    

    def save_file(self):
        if self.bank is None:
            messagebox.showerror("Error", "Load a client file first.")
            return

        filename = filedialog.asksaveasfilename(
            title="Save Client CSV File",
            defaultextension=".csv",
            filetypes=[("CSV Files", "*.csv")],
            initialfile="clients_saved.csv"
        )

        if not filename:
            return

        error = ctypes.create_string_buffer(512)

        success = bank_lib.bank_save(
            self.bank,
            filename.encode("utf-8"),
            error,
            len(error)
        )

        if not success:
            messagebox.showerror(
                "Save Error",
                error.value.decode("utf-8", errors="replace")
            )
            return

        messagebox.showinfo(
            "Success",
            "Client information saved successfully!"
        )



if __name__ == "__main__":
    root = tk.Tk()
    app = BankApp(root)
    root.mainloop()
