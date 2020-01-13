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
    /// SVectorSet.xaml 的交互逻辑
    /// </summary>
    public partial class SVectorSet : UserControl, ISPropertyInterface
    {
        public SakuraCore.SPropertyData PropertyData { get; set; }
        public SVectorSet()
        {
            InitializeComponent();
            
        }

        public void Initialize(SakuraCore.SPropertyData property)
        {
            PropertyData = property;
            ParamName.Text = property.PropName;
            SakuraCore.SVector val;
            SakuraCore.GetSVectorProp(property.SObject, property.PropName, out val, property.SourceType);
            ValueBoxX.Text = val.x.ToString(); 
            ValueBoxY.Text = val.y.ToString(); 
            ValueBoxZ.Text = val.z.ToString(); 
        }

        public void UpdateValue()
        {
            try
            {
                SakuraCore.SVector val = new SakuraCore.SVector();
                val.x = float.Parse(ValueBoxX.Text);
                val.y = float.Parse(ValueBoxY.Text);
                val.z = float.Parse(ValueBoxZ.Text);
                SakuraCore.SetSVectorProp(PropertyData.SObject, PropertyData.PropName, val);
            }
            catch
            {

            }
        }

        private void ValueBox_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            var box = sender as TextBox;
            if(e.Key == Key.Enter)
            {
                box.MoveFocus(new TraversalRequest(FocusNavigationDirection.Next));
                UpdateValue();
            }
        }
    }
}
