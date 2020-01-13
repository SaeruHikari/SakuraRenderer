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
    /// StringSet.xaml 的交互逻辑
    /// </summary>
    public partial class StringSet : UserControl, ISPropertyInterface
    {
        public SakuraCore.SPropertyData PropertyData { get; set; }
        public StringSet()
        {
            InitializeComponent();
        }

        public void Initialize(SakuraCore.SPropertyData property)
        {
            PropertyData = property;
            ParamName.Text = property.PropName;
            string val = SakuraCore.GetStringProp(property.SObject, property.PropName, property.SourceType);
            ValueBox.Text = val;
        }

        public void UpdateValue()
        {

        }

        private void ValueBox_TextChanged(object sender, TextChangedEventArgs e)
        {

        }
    }
}
