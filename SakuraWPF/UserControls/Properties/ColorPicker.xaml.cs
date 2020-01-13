using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace SakuraWPF.UserControls.Properties
{
    /// <summary>
    /// ColorPicker.xaml 的交互逻辑
    /// </summary>
    public partial class ColorPicker : UserControl, ISPropertyInterface
    {
        public SakuraCore.SPropertyData PropertyData { get; set; }
        public ColorPicker()
        {
            InitializeComponent();
        }

        public void Initialize(SakuraCore.SPropertyData property)
        {
            throw new NotImplementedException();
        }

        public void UpdateValue()
        {
            throw new NotImplementedException();
        }
    }
}
