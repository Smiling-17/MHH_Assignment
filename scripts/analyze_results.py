#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
===============================================================================
  PETRI NET SOLVER - RESULTS ANALYZER
  Script phân tích kết quả và vẽ biểu đồ cho báo cáo
  
  Author: Group 90
  Course: CO2011 - Mathematical Modeling
  University: HCMUT
===============================================================================
"""

import os
import sys
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from datetime import datetime

# ============================================================================
# CONFIGURATION
# ============================================================================

# Paths
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
OUTPUT_DIR = os.path.join(PROJECT_ROOT, "output")
DATA_DIR = os.path.join(PROJECT_ROOT, "data")

# Files
CSV_FILE = os.path.join(OUTPUT_DIR, "result.csv")
DEADLOCK_FILE = os.path.join(OUTPUT_DIR, "deadlock.txt")
OPTIMUM_FILE = os.path.join(OUTPUT_DIR, "optimum.txt")

# Style configuration
plt.style.use('seaborn-v0_8-whitegrid')
COLORS = {
    'explicit': '#3498db',  # Blue
    'bdd': '#e74c3c',       # Red
    'ilp': '#2ecc71',       # Green
    'accent': '#9b59b6',    # Purple
    'background': '#f8f9fa',
    'text': '#2c3e50'
}

FONT_CONFIG = {
    'family': 'sans-serif',
    'weight': 'normal',
    'size': 11
}
plt.rc('font', **FONT_CONFIG)

# ============================================================================
# DATA LOADING
# ============================================================================

def load_results():
    """Load results from CSV file"""
    if not os.path.exists(CSV_FILE):
        print(f"❌ Error: Result file not found: {CSV_FILE}")
        print("   Please run the petri_solver first!")
        return None
    
    try:
        df = pd.read_csv(CSV_FILE)
        print(f"✅ Loaded {len(df)} records from {CSV_FILE}")
        return df
    except Exception as e:
        print(f"❌ Error loading CSV: {e}")
        return None

def load_deadlock_info():
    """Load deadlock detection results"""
    if os.path.exists(DEADLOCK_FILE):
        with open(DEADLOCK_FILE, 'r') as f:
            return f.read().strip()
    return "No deadlock info"

def load_optimum_info():
    """Load optimization results"""
    if os.path.exists(OPTIMUM_FILE):
        with open(OPTIMUM_FILE, 'r') as f:
            return f.read().strip()
    return "No optimization info"

# ============================================================================
# TABLE GENERATION
# ============================================================================

def print_table(df, title="Results"):
    """Print a formatted table to console"""
    print("\n" + "=" * 80)
    print(f"  📊 {title}")
    print("=" * 80)
    
    # Header
    cols = df.columns.tolist()
    widths = [max(len(str(col)), df[col].astype(str).str.len().max()) + 2 for col in cols]
    
    # Print header
    header = "│"
    for col, w in zip(cols, widths):
        header += f" {col:^{w}} │"
    
    separator = "├" + "┼".join(["─" * (w + 2) for w in widths]) + "┤"
    top_border = "┌" + "┬".join(["─" * (w + 2) for w in widths]) + "┐"
    bottom_border = "└" + "┴".join(["─" * (w + 2) for w in widths]) + "┘"
    
    print(top_border)
    print(header)
    print(separator)
    
    # Print rows
    for _, row in df.iterrows():
        row_str = "│"
        for col, w in zip(cols, widths):
            val = row[col]
            if isinstance(val, float):
                val = f"{val:.6f}" if val < 0.01 else f"{val:.4f}"
            row_str += f" {str(val):^{w}} │"
        print(row_str)
    
    print(bottom_border)
    print()

def generate_latex_table(df, caption="Performance Comparison"):
    """Generate LaTeX table code"""
    latex = "\\begin{table}[htbp]\n"
    latex += "\\centering\n"
    latex += f"\\caption{{{caption}}}\n"
    latex += "\\begin{tabular}{|" + "c|" * len(df.columns) + "}\n"
    latex += "\\hline\n"
    
    # Header
    latex += " & ".join([f"\\textbf{{{col}}}" for col in df.columns]) + " \\\\\n"
    latex += "\\hline\n"
    
    # Rows
    for _, row in df.iterrows():
        vals = []
        for col in df.columns:
            val = row[col]
            if isinstance(val, float):
                vals.append(f"{val:.4f}")
            else:
                vals.append(str(val))
        latex += " & ".join(vals) + " \\\\\n"
    
    latex += "\\hline\n"
    latex += "\\end{tabular}\n"
    latex += "\\label{tab:performance}\n"
    latex += "\\end{table}\n"
    
    return latex

# ============================================================================
# CHART GENERATION
# ============================================================================

def create_performance_comparison_chart(df, save_path=None):
    """
    Create bar chart comparing Explicit vs BDD performance
    """
    fig, axes = plt.subplots(1, 3, figsize=(15, 5))
    fig.suptitle('Performance Comparison: Explicit vs BDD', fontsize=16, fontweight='bold', y=1.02)
    
    # Group by model
    models = df['Model'].unique()
    
    # Prepare data
    explicit_data = df[df['Method'] == 'Explicit']
    bdd_data = df[df['Method'] == 'BDD']
    
    x = np.arange(len(models))
    width = 0.35
    
    # Chart 1: States (should be equal)
    ax1 = axes[0]
    explicit_states = [explicit_data[explicit_data['Model'] == m]['States'].values[0] 
                       if len(explicit_data[explicit_data['Model'] == m]) > 0 else 0 
                       for m in models]
    bdd_states = [bdd_data[bdd_data['Model'] == m]['States'].values[0] 
                  if len(bdd_data[bdd_data['Model'] == m]) > 0 else 0 
                  for m in models]
    
    bars1 = ax1.bar(x - width/2, explicit_states, width, label='Explicit', color=COLORS['explicit'], edgecolor='white')
    bars2 = ax1.bar(x + width/2, bdd_states, width, label='BDD', color=COLORS['bdd'], edgecolor='white')
    
    ax1.set_xlabel('Model', fontsize=12)
    ax1.set_ylabel('Number of States', fontsize=12)
    ax1.set_title('Reachable States Count', fontsize=14, fontweight='bold')
    ax1.set_xticks(x)
    ax1.set_xticklabels([m.replace('.pnml', '') for m in models], rotation=45, ha='right')
    ax1.legend()
    ax1.grid(axis='y', alpha=0.3)
    
    # Add value labels
    for bar in bars1:
        height = bar.get_height()
        ax1.annotate(f'{int(height)}', xy=(bar.get_x() + bar.get_width()/2, height),
                    xytext=(0, 3), textcoords="offset points", ha='center', va='bottom', fontsize=9)
    for bar in bars2:
        height = bar.get_height()
        ax1.annotate(f'{int(height)}', xy=(bar.get_x() + bar.get_width()/2, height),
                    xytext=(0, 3), textcoords="offset points", ha='center', va='bottom', fontsize=9)
    
    # Chart 2: Time comparison
    ax2 = axes[1]
    explicit_time = [explicit_data[explicit_data['Model'] == m]['TimeSec'].values[0] 
                     if len(explicit_data[explicit_data['Model'] == m]) > 0 else 0 
                     for m in models]
    bdd_time = [bdd_data[bdd_data['Model'] == m]['TimeSec'].values[0] 
                if len(bdd_data[bdd_data['Model'] == m]) > 0 else 0 
                for m in models]
    
    bars3 = ax2.bar(x - width/2, explicit_time, width, label='Explicit', color=COLORS['explicit'], edgecolor='white')
    bars4 = ax2.bar(x + width/2, bdd_time, width, label='BDD', color=COLORS['bdd'], edgecolor='white')
    
    ax2.set_xlabel('Model', fontsize=12)
    ax2.set_ylabel('Time (seconds)', fontsize=12)
    ax2.set_title('Execution Time', fontsize=14, fontweight='bold')
    ax2.set_xticks(x)
    ax2.set_xticklabels([m.replace('.pnml', '') for m in models], rotation=45, ha='right')
    ax2.legend()
    ax2.grid(axis='y', alpha=0.3)
    
    # Chart 3: Memory comparison
    ax3 = axes[2]
    explicit_mem = [explicit_data[explicit_data['Model'] == m]['MemMB'].values[0] 
                    if len(explicit_data[explicit_data['Model'] == m]) > 0 else 0 
                    for m in models]
    bdd_mem = [bdd_data[bdd_data['Model'] == m]['MemMB'].values[0] 
               if len(bdd_data[bdd_data['Model'] == m]) > 0 else 0 
               for m in models]
    
    bars5 = ax3.bar(x - width/2, explicit_mem, width, label='Explicit', color=COLORS['explicit'], edgecolor='white')
    bars6 = ax3.bar(x + width/2, bdd_mem, width, label='BDD', color=COLORS['bdd'], edgecolor='white')
    
    ax3.set_xlabel('Model', fontsize=12)
    ax3.set_ylabel('Memory (MB)', fontsize=12)
    ax3.set_title('Memory Usage', fontsize=14, fontweight='bold')
    ax3.set_xticks(x)
    ax3.set_xticklabels([m.replace('.pnml', '') for m in models], rotation=45, ha='right')
    ax3.legend()
    ax3.grid(axis='y', alpha=0.3)
    
    plt.tight_layout()
    
    if save_path:
        plt.savefig(save_path, dpi=300, bbox_inches='tight', facecolor='white')
        print(f"📈 Chart saved: {save_path}")
    
    return fig

def create_speedup_chart(df, save_path=None):
    """
    Create chart showing speedup of BDD over Explicit
    """
    fig, ax = plt.subplots(figsize=(10, 6))
    
    models = df['Model'].unique()
    explicit_data = df[df['Method'] == 'Explicit']
    bdd_data = df[df['Method'] == 'BDD']
    
    speedups = []
    model_names = []
    
    for m in models:
        exp_time = explicit_data[explicit_data['Model'] == m]['TimeSec'].values
        bdd_time = bdd_data[bdd_data['Model'] == m]['TimeSec'].values
        
        if len(exp_time) > 0 and len(bdd_time) > 0 and bdd_time[0] > 0:
            speedup = exp_time[0] / bdd_time[0]
            speedups.append(speedup)
            model_names.append(m.replace('.pnml', ''))
    
    if len(speedups) == 0:
        print("⚠️  Not enough data to create speedup chart")
        return None
    
    colors = [COLORS['bdd'] if s > 1 else COLORS['explicit'] for s in speedups]
    bars = ax.bar(model_names, speedups, color=colors, edgecolor='white', linewidth=1.5)
    
    # Add horizontal line at y=1
    ax.axhline(y=1, color='gray', linestyle='--', linewidth=2, label='No speedup (1x)')
    
    ax.set_xlabel('Model', fontsize=12)
    ax.set_ylabel('Speedup (Explicit Time / BDD Time)', fontsize=12)
    ax.set_title('BDD Speedup over Explicit Method', fontsize=14, fontweight='bold')
    ax.legend()
    ax.grid(axis='y', alpha=0.3)
    
    # Add value labels
    for bar, val in zip(bars, speedups):
        height = bar.get_height()
        label = f'{val:.2f}x'
        ax.annotate(label, xy=(bar.get_x() + bar.get_width()/2, height),
                   xytext=(0, 3), textcoords="offset points", ha='center', va='bottom', fontsize=10, fontweight='bold')
    
    # Add annotation
    faster_bdd = sum(1 for s in speedups if s > 1)
    faster_exp = len(speedups) - faster_bdd
    ax.text(0.02, 0.98, f'BDD faster: {faster_bdd} models\nExplicit faster: {faster_exp} models',
            transform=ax.transAxes, fontsize=10, verticalalignment='top',
            bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
    
    plt.tight_layout()
    
    if save_path:
        plt.savefig(save_path, dpi=300, bbox_inches='tight', facecolor='white')
        print(f"📈 Chart saved: {save_path}")
    
    return fig

def create_summary_dashboard(df, save_path=None):
    """
    Create a comprehensive dashboard with multiple charts
    """
    fig = plt.figure(figsize=(16, 12))
    fig.suptitle('Petri Net Solver - Performance Dashboard', fontsize=18, fontweight='bold', y=0.98)
    
    # Create grid
    gs = fig.add_gridspec(3, 3, hspace=0.35, wspace=0.3)
    
    models = df['Model'].unique()
    explicit_data = df[df['Method'] == 'Explicit']
    bdd_data = df[df['Method'] == 'BDD']
    
    # =========== Chart 1: States Bar Chart ===========
    ax1 = fig.add_subplot(gs[0, 0])
    x = np.arange(len(models))
    width = 0.35
    
    exp_states = [explicit_data[explicit_data['Model'] == m]['States'].values[0] 
                  if len(explicit_data[explicit_data['Model'] == m]) > 0 else 0 for m in models]
    bdd_states = [bdd_data[bdd_data['Model'] == m]['States'].values[0] 
                  if len(bdd_data[bdd_data['Model'] == m]) > 0 else 0 for m in models]
    
    ax1.bar(x - width/2, exp_states, width, label='Explicit', color=COLORS['explicit'])
    ax1.bar(x + width/2, bdd_states, width, label='BDD', color=COLORS['bdd'])
    ax1.set_title('Reachable States', fontweight='bold')
    ax1.set_xticks(x)
    ax1.set_xticklabels([m.replace('.pnml', '')[:10] for m in models], rotation=45, ha='right', fontsize=8)
    ax1.legend(fontsize=8)
    ax1.grid(axis='y', alpha=0.3)
    
    # =========== Chart 2: Time Bar Chart ===========
    ax2 = fig.add_subplot(gs[0, 1])
    
    exp_time = [explicit_data[explicit_data['Model'] == m]['TimeSec'].values[0] 
                if len(explicit_data[explicit_data['Model'] == m]) > 0 else 0 for m in models]
    bdd_time = [bdd_data[bdd_data['Model'] == m]['TimeSec'].values[0] 
                if len(bdd_data[bdd_data['Model'] == m]) > 0 else 0 for m in models]
    
    ax2.bar(x - width/2, exp_time, width, label='Explicit', color=COLORS['explicit'])
    ax2.bar(x + width/2, bdd_time, width, label='BDD', color=COLORS['bdd'])
    ax2.set_title('Execution Time (s)', fontweight='bold')
    ax2.set_xticks(x)
    ax2.set_xticklabels([m.replace('.pnml', '')[:10] for m in models], rotation=45, ha='right', fontsize=8)
    ax2.legend(fontsize=8)
    ax2.grid(axis='y', alpha=0.3)
    
    # =========== Chart 3: Pie Chart - Method Distribution ===========
    ax3 = fig.add_subplot(gs[0, 2])
    method_counts = df['Method'].value_counts()
    colors_pie = [COLORS['explicit'], COLORS['bdd']]
    ax3.pie(method_counts.values, labels=method_counts.index, autopct='%1.1f%%',
            colors=colors_pie, startangle=90, explode=[0.05]*len(method_counts))
    ax3.set_title('Method Distribution', fontweight='bold')
    
    # =========== Chart 4: Time Comparison Line ===========
    ax4 = fig.add_subplot(gs[1, :2])
    
    model_short = [m.replace('.pnml', '') for m in models]
    ax4.plot(model_short, exp_time, 'o-', label='Explicit', color=COLORS['explicit'], linewidth=2, markersize=8)
    ax4.plot(model_short, bdd_time, 's-', label='BDD', color=COLORS['bdd'], linewidth=2, markersize=8)
    ax4.fill_between(model_short, exp_time, alpha=0.3, color=COLORS['explicit'])
    ax4.fill_between(model_short, bdd_time, alpha=0.3, color=COLORS['bdd'])
    ax4.set_xlabel('Model')
    ax4.set_ylabel('Time (seconds)')
    ax4.set_title('Execution Time Comparison', fontweight='bold')
    ax4.legend()
    ax4.grid(True, alpha=0.3)
    plt.setp(ax4.xaxis.get_majorticklabels(), rotation=45, ha='right')
    
    # =========== Chart 5: Deadlock Info ===========
    ax5 = fig.add_subplot(gs[1, 2])
    ax5.axis('off')
    
    deadlock_info = load_deadlock_info()
    optimum_info = load_optimum_info()
    
    info_text = "📋 ANALYSIS RESULTS\n"
    info_text += "─" * 30 + "\n\n"
    info_text += "🔴 Deadlock Detection:\n"
    info_text += f"   {deadlock_info}\n\n"
    info_text += "🟢 Optimization:\n"
    info_text += f"   {optimum_info}\n\n"
    info_text += "─" * 30 + "\n"
    info_text += f"📅 Generated: {datetime.now().strftime('%Y-%m-%d %H:%M')}"
    
    ax5.text(0.1, 0.9, info_text, transform=ax5.transAxes, fontsize=10,
             verticalalignment='top', fontfamily='monospace',
             bbox=dict(boxstyle='round', facecolor='lightyellow', alpha=0.8))
    
    # =========== Chart 6: Summary Statistics ===========
    ax6 = fig.add_subplot(gs[2, 0])
    ax6.axis('off')
    
    # Calculate statistics
    total_states_exp = sum(exp_states)
    total_states_bdd = sum(bdd_states)
    total_time_exp = sum(exp_time)
    total_time_bdd = sum(bdd_time)
    avg_speedup = total_time_exp / total_time_bdd if total_time_bdd > 0 else 0
    
    stats_text = "📊 SUMMARY STATISTICS\n"
    stats_text += "─" * 25 + "\n\n"
    stats_text += f"Total Models: {len(models)}\n\n"
    stats_text += "Explicit Method:\n"
    stats_text += f"  • Total States: {total_states_exp}\n"
    stats_text += f"  • Total Time: {total_time_exp:.4f}s\n\n"
    stats_text += "BDD Method:\n"
    stats_text += f"  • Total States: {total_states_bdd}\n"
    stats_text += f"  • Total Time: {total_time_bdd:.4f}s\n\n"
    stats_text += f"Avg Speedup: {avg_speedup:.2f}x"
    
    ax6.text(0.1, 0.9, stats_text, transform=ax6.transAxes, fontsize=10,
             verticalalignment='top', fontfamily='monospace',
             bbox=dict(boxstyle='round', facecolor='lightcyan', alpha=0.8))
    
    # =========== Chart 7: Stacked Bar - States by Model ===========
    ax7 = fig.add_subplot(gs[2, 1])
    
    # Deadlock status
    deadlock_status = []
    for m in models:
        bdd_row = bdd_data[bdd_data['Model'] == m]
        if len(bdd_row) > 0:
            dl = bdd_row['Deadlock'].values[0]
            deadlock_status.append('Yes' if dl == 'Yes' else 'No')
        else:
            deadlock_status.append('N/A')
    
    colors_dl = ['#e74c3c' if s == 'Yes' else '#2ecc71' if s == 'No' else '#95a5a6' for s in deadlock_status]
    bars = ax7.bar(model_short, bdd_states, color=colors_dl, edgecolor='white')
    ax7.set_title('States & Deadlock Status', fontweight='bold')
    ax7.set_xlabel('Model')
    ax7.set_ylabel('States')
    plt.setp(ax7.xaxis.get_majorticklabels(), rotation=45, ha='right', fontsize=8)
    
    # Legend for deadlock
    legend_elements = [
        mpatches.Patch(facecolor='#e74c3c', label='Has Deadlock'),
        mpatches.Patch(facecolor='#2ecc71', label='No Deadlock'),
    ]
    ax7.legend(handles=legend_elements, fontsize=8)
    ax7.grid(axis='y', alpha=0.3)
    
    # =========== Chart 8: Methods Comparison Radar (simplified) ===========
    ax8 = fig.add_subplot(gs[2, 2])
    
    # Create simple comparison metrics
    metrics = ['Speed', 'Memory\nEfficiency', 'Scalability']
    
    # Normalized scores (example - you can adjust based on actual data)
    if total_time_bdd > 0 and total_time_exp > 0:
        exp_scores = [1.0, 0.5, 0.3]  # Explicit scores
        bdd_scores = [min(total_time_exp/total_time_bdd, 2.0)/2.0, 0.8, 0.9]  # BDD scores
    else:
        exp_scores = [0.5, 0.5, 0.3]
        bdd_scores = [0.7, 0.8, 0.9]
    
    x_pos = np.arange(len(metrics))
    width = 0.35
    
    ax8.barh(x_pos - width/2, exp_scores, width, label='Explicit', color=COLORS['explicit'])
    ax8.barh(x_pos + width/2, bdd_scores, width, label='BDD', color=COLORS['bdd'])
    ax8.set_yticks(x_pos)
    ax8.set_yticklabels(metrics)
    ax8.set_xlabel('Score (normalized)')
    ax8.set_title('Method Comparison', fontweight='bold')
    ax8.legend(fontsize=8)
    ax8.set_xlim(0, 1.2)
    ax8.grid(axis='x', alpha=0.3)
    
    plt.tight_layout()
    
    if save_path:
        plt.savefig(save_path, dpi=300, bbox_inches='tight', facecolor='white')
        print(f"📈 Dashboard saved: {save_path}")
    
    return fig

def create_scalability_analysis(df, save_path=None):
    """
    Create scalability analysis chart
    """
    fig, axes = plt.subplots(1, 2, figsize=(14, 5))
    fig.suptitle('Scalability Analysis', fontsize=16, fontweight='bold')
    
    models = df['Model'].unique()
    bdd_data = df[df['Method'] == 'BDD']
    explicit_data = df[df['Method'] == 'Explicit']
    
    # Get states and times
    states_list = []
    bdd_times = []
    exp_times = []
    
    for m in models:
        bdd_row = bdd_data[bdd_data['Model'] == m]
        exp_row = explicit_data[explicit_data['Model'] == m]
        
        if len(bdd_row) > 0 and len(exp_row) > 0:
            states_list.append(bdd_row['States'].values[0])
            bdd_times.append(bdd_row['TimeSec'].values[0])
            exp_times.append(exp_row['TimeSec'].values[0])
    
    if len(states_list) < 2:
        print("⚠️  Not enough data for scalability analysis")
        return None
    
    # Sort by states
    sorted_idx = np.argsort(states_list)
    states_sorted = [states_list[i] for i in sorted_idx]
    bdd_sorted = [bdd_times[i] for i in sorted_idx]
    exp_sorted = [exp_times[i] for i in sorted_idx]
    
    # Chart 1: Time vs States
    ax1 = axes[0]
    ax1.plot(states_sorted, exp_sorted, 'o-', label='Explicit', color=COLORS['explicit'], linewidth=2, markersize=10)
    ax1.plot(states_sorted, bdd_sorted, 's-', label='BDD', color=COLORS['bdd'], linewidth=2, markersize=10)
    ax1.set_xlabel('Number of States', fontsize=12)
    ax1.set_ylabel('Time (seconds)', fontsize=12)
    ax1.set_title('Execution Time vs State Space Size', fontweight='bold')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # Chart 2: Log scale
    ax2 = axes[1]
    ax2.semilogy(states_sorted, exp_sorted, 'o-', label='Explicit', color=COLORS['explicit'], linewidth=2, markersize=10)
    ax2.semilogy(states_sorted, bdd_sorted, 's-', label='BDD', color=COLORS['bdd'], linewidth=2, markersize=10)
    ax2.set_xlabel('Number of States', fontsize=12)
    ax2.set_ylabel('Time (seconds) - Log Scale', fontsize=12)
    ax2.set_title('Execution Time (Log Scale)', fontweight='bold')
    ax2.legend()
    ax2.grid(True, alpha=0.3, which='both')
    
    plt.tight_layout()
    
    if save_path:
        plt.savefig(save_path, dpi=300, bbox_inches='tight', facecolor='white')
        print(f"📈 Chart saved: {save_path}")
    
    return fig

# ============================================================================
# MAIN EXECUTION
# ============================================================================

def main():
    print("=" * 60)
    print("  🔬 PETRI NET SOLVER - RESULTS ANALYZER")
    print("  📊 Generating Tables and Charts for Report")
    print("=" * 60)
    print()
    
    # Load data
    df = load_results()
    
    if df is None or len(df) == 0:
        print("\n⚠️  No data to analyze. Please run the solver first:")
        print("    ./petri_solver --input data/simple_test.pnml --mode all --optimize")
        return
    
    # Print tables
    print("\n" + "=" * 60)
    print("  📋 TABLES")
    print("=" * 60)
    
    # Performance comparison table
    comparison_df = df[['Model', 'Method', 'States', 'TimeSec', 'MemMB']].copy()
    print_table(comparison_df, "Performance Comparison (Explicit vs BDD)")
    
    # Deadlock & Optimization table
    bdd_df = df[df['Method'] == 'BDD'][['Model', 'States', 'Deadlock', 'OptObj', 'OptMarking']].copy()
    if len(bdd_df) > 0:
        print_table(bdd_df, "Deadlock Detection & Optimization Results")
    
    # Generate LaTeX
    print("\n📄 LaTeX Table Code:")
    print("-" * 40)
    latex_code = generate_latex_table(comparison_df, "Performance Comparison: Explicit vs BDD")
    print(latex_code)
    
    # Save LaTeX to file
    latex_path = os.path.join(OUTPUT_DIR, "table_performance.tex")
    with open(latex_path, 'w') as f:
        f.write(latex_code)
    print(f"📄 LaTeX saved: {latex_path}")
    
    # Generate charts
    print("\n" + "=" * 60)
    print("  📈 GENERATING CHARTS")
    print("=" * 60)
    
    # Chart 1: Performance comparison
    chart1_path = os.path.join(OUTPUT_DIR, "chart_performance_comparison.png")
    create_performance_comparison_chart(df, chart1_path)
    
    # Chart 2: Speedup
    chart2_path = os.path.join(OUTPUT_DIR, "chart_speedup.png")
    create_speedup_chart(df, chart2_path)
    
    # Chart 3: Dashboard
    chart3_path = os.path.join(OUTPUT_DIR, "chart_dashboard.png")
    create_summary_dashboard(df, chart3_path)
    
    # Chart 4: Scalability
    chart4_path = os.path.join(OUTPUT_DIR, "chart_scalability.png")
    create_scalability_analysis(df, chart4_path)
    
    # Summary
    print("\n" + "=" * 60)
    print("  ✅ ANALYSIS COMPLETE")
    print("=" * 60)
    print(f"\n📁 Output directory: {OUTPUT_DIR}")
    print("\n📊 Generated files:")
    print(f"   • table_performance.tex")
    print(f"   • chart_performance_comparison.png")
    print(f"   • chart_speedup.png")
    print(f"   • chart_dashboard.png")
    print(f"   • chart_scalability.png")
    print("\n💡 Use these in your report (≤15 pages)")
    print()
    
    # Show charts if running interactively
    try:
        plt.show()
    except:
        pass

if __name__ == "__main__":
    main()



