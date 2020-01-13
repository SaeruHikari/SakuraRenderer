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
using SakuraWPF.UserControls.Properties;
namespace SakuraWPF.UserControls
{
    /// <summary>
    /// DetailView.xaml 的交互逻辑
    /// </summary>
    public partial class DetailView : UserControl, ISPropertyInterface
    {
        IntPtr ActiveNode = IntPtr.Zero;
        List<ISPropertyInterface> PropertySetters = new List<ISPropertyInterface>();

        public SakuraCore.SPropertyData PropertyData { get; set; }

        public DetailView()
        {
            InitializeComponent();

        }

        public void RefreshActiveNode(IntPtr NewNode)
        {
            ActiveNode = NewNode;
            SceneViewTab.Header = SakuraCore.GetStringProp(NewNode, "Name");
            var props = SakuraCore.GetObjectProperties(NewNode);
            PropertySetters.Clear();
            DetailsDock.Children.Clear();
            foreach (var prop in props)
            {
                RegistNewSetter(prop);
            }
        }
        public ISPropertyInterface RegistSubMember(SakuraCore.SPropertyData submem)
        {
            ISPropertyInterface newset = new DetailView();
            IntPtr memobj;
            SakuraCore.GetSubmemberProp(ActiveNode, submem.PropName, out memobj, submem.SourceType);
            if(memobj != IntPtr.Zero)
                (newset as DetailView).RefreshActiveNode(memobj);
            return newset;
        }
        public void RegistContainer(SakuraCore.SPropertyData container)
        {
            return;
        }

        public void RegistNewSetter(SakuraCore.SPropertyData property)
        {
            ISPropertyInterface newset;
            switch (property.PropType)
            {
                case "bool":
                    {
                        newset = new BoolSwitch() as ISPropertyInterface; break;
                    }
                case "float":
                    {
                        newset = new FloatSet() as ISPropertyInterface; break;
                    }
                case "const SVector&":
                case "SVector":
                case "structSakuraMath::SVector":
                    {
                        newset = new SVectorSet() as ISPropertyInterface;break;
                    }
                case "const std::string&":
                case "std::string":
                case "string":
                case "std:string":
                    {
                        newset = new StringSet() as ISPropertyInterface; break;
                    }
                default:
                    {
                        // Containers
                        if (SakuraCore.PropIsContainer(property))
                        {
                            RegistContainer(property);
                            return;
                        }
                        else
                        {
                            // 
                            newset = RegistSubMember(property);
                            break;
                        }
                    }
            }
            newset.Initialize(property);
            PropertySetters.Add(newset);
            DetailsDock.Children.Add(newset as UserControl);
            (newset as UserControl).SetValue(DockPanel.DockProperty, Dock.Top);
        }

        public void Initialize(SakuraCore.SPropertyData property)
        {
            return;
        }

        public void UpdateValue()
        {
            return;
        }
    }
}
