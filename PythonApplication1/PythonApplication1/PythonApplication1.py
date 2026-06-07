import tkinter as tk
from tkinter import ttk, messagebox
import pandas as pd
import datetime
import io
import requests
import os

class ExchangeRateApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Currency Exchange Rate Downloader (Stooq Direct)")
        self.root.geometry("500x380")
        
        # Style configuration
        self.style = ttk.Style()
        self.style.configure("TButton", font=("Helvetica", 12))
        self.style.configure("TLabel", font=("Helvetica", 11))
        
        # UI Elements
        self.create_widgets()
        
    def create_widgets(self):
        title_label = ttk.Label(self.root, text="Historical Exchange Rates to CNY", font=("Helvetica", 16, "bold"))
        title_label.pack(pady=20)
        
        desc_label = ttk.Label(self.root, text="Source: Stooq (Direct CSV Download)\nCurrencies: TWD, GBP, JPY, USD, EUR, HKD\nFix: No pandas-datareader dependency", justify="center")
        desc_label.pack(pady=10)
        
        self.log_text = tk.Text(self.root, height=8, width=55, state='disabled', bg="#f0f0f0", font=("Consolas", 8))
        self.log_text.pack(pady=10)
        
        self.download_btn = ttk.Button(self.root, text="Download & Generate Excel", command=self.run_process)
        self.download_btn.pack(pady=10)
        
        footer = ttk.Label(self.root, text="Output file: exchange_rates_2024.xlsx", font=("Helvetica", 9, "italic"))
        footer.pack(pady=5)

    def log(self, message):
        self.log_text.config(state='normal')
        self.log_text.insert(tk.END, f">> {message}\n")
        self.log_text.see(tk.END)
        self.log_text.config(state='disabled')
        self.root.update()

    def get_stooq_data(self, symbol):
        """
        Directly fetches CSV from Stooq to avoid pandas-datareader bugs.
        """
        url = f"https://stooq.com/q/d/l/?s={symbol}&i=d"
        try:
            # Stooq sometimes requires a User-Agent to prevent bot blocking
            headers = {
                "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36"
            }
            response = requests.get(url, headers=headers)
            response.raise_for_status()
            
            # Read CSV from the response content
            df = pd.read_csv(io.StringIO(response.text))
            
            # Stooq CSV columns are usually: Date, Open, High, Low, Close
            # We need to ensure 'Date' is datetime and set as index
            df['Date'] = pd.to_datetime(df['Date'])
            df.set_index('Date', inplace=True)
            
            # Filter for 2023 onwards
            start_date = "2023-01-01"
            df = df[df.index >= start_date]
            
            return df['Close']
            
        except Exception as e:
            self.log(f"Error fetching {symbol}: {str(e)}")
            return None

    def fetch_data(self):
        # Dictionary mapping: Name -> Stooq Symbol
        tickers = {
            'USD_CNY': 'USDCNY',
            'USD_TWD': 'USDTWD', 
            'GBP_USD': 'GBPUSD',
            'USD_JPY': 'USDJPY',
            'EUR_USD': 'EURUSD',
            'USD_HKD': 'USDHKD'
        }
        
        data_frames = {}
        
        for name, symbol in tickers.items():
            self.log(f"Fetching {name} ({symbol})...")
            series = self.get_stooq_data(symbol)
            if series is not None and not series.empty:
                data_frames[name] = series
            else:
                self.log(f"Warning: No data found for {name}")
        
        if not data_frames:
            return None
            
        self.log("Data fetch complete. Merging...")
        return pd.DataFrame(data_frames)

    def calculate_cross_rates(self, df):
        result = pd.DataFrame(index=df.index)
        
        # Ensure we have the necessary base pairs
        if 'USD_CNY' not in df.columns:
            messagebox.showerror("Error", "Critical data (USD_CNY) missing from source.")
            return None

        # 1. USD to CNY
        result['USD/CNY'] = df['USD_CNY']
        
        # 2. TWD to CNY (Formula: USD_CNY / USD_TWD)
        if 'USD_TWD' in df.columns:
            result['TWD/CNY'] = df['USD_CNY'] / df['USD_TWD']
        
        # 3. GBP to CNY (Formula: GBP_USD * USD_CNY)
        if 'GBP_USD' in df.columns:
            result['GBP/CNY'] = df['GBP_USD'] * df['USD_CNY']
        
        # 4. EUR to CNY (Formula: EUR_USD * USD_CNY)
        if 'EUR_USD' in df.columns:
            result['EUR/CNY'] = df['EUR_USD'] * df['USD_CNY']
        
        # 5. JPY to CNY (Formula: USD_CNY / USD_JPY)
        if 'USD_JPY' in df.columns:
            result['JPY/CNY'] = df['USD_CNY'] / df['USD_JPY']
        
        # 6. HKD to CNY (Formula: USD_CNY / USD_HKD)
        if 'USD_HKD' in df.columns:
            result['HKD/CNY'] = df['USD_CNY'] / df['USD_HKD']
        
        # Clean up
        result = result.sort_index(ascending=True) # Oldest date first
        result = result.round(4)
        return result

    def run_process(self):
        self.download_btn.config(state='disabled')
        self.log_text.config(state='normal')
        self.log_text.delete(1.0, tk.END)
        self.log_text.config(state='disabled')
        
        raw_df = self.fetch_data()
        
        if raw_df is not None:
            self.log("Calculating cross rates...")
            final_df = self.calculate_cross_rates(raw_df)
            
            if final_df is not None:
                # Remove rows where all data is NaN (optional)
                final_df = final_df.dropna(how='all')
                
                output_file = "exchange_rates_2024.xlsx"
                try:
                    self.log(f"Saving to {output_file}...")
                    final_df.to_excel(output_file)
                    self.log("Success! File generated.")
                    messagebox.showinfo("Success", f"Data saved successfully to:\n{os.path.abspath(output_file)}")
                except Exception as e:
                    messagebox.showerror("File Error", f"Could not save file.\n{e}")
        else:
            messagebox.showerror("Error", "Failed to retrieve any data. Please check your internet connection.")
            
        self.download_btn.config(state='normal')

if __name__ == "__main__":
    root = tk.Tk()
    app = ExchangeRateApp(root)
    root.mainloop()