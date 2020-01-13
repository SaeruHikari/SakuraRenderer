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

namespace SakuraWPF.UserControls
{
    /// <summary>
    /// SceneView.xaml 的交互逻辑
    /// </summary>
    public partial class SceneView : UserControl
    {
        public IntPtr SceneRootNode = IntPtr.Zero;
        public IntPtr ActiveNode = IntPtr.Zero;
        public SceneNodeProp RootProp;
        MainWindow mainWindow;
        public SceneView(MainWindow mainWind)
        {
            InitializeComponent();
            mainWindow = mainWind;
            SceneViewTree.SelectedItemChanged += SceneViewTree_SelectedItemChanged;
            SceneViewTree.SelectedItemChanged += mainWind.SceneViewTree_SelectedItemChanged;
        }

        public void InitializeSceneView()
        {
            SceneRootNode = SakuraCore.GetSceneNode(IntPtr.Zero);
            ActiveNode = SakuraCore.GetSceneNode(IntPtr.Zero);
            SakuraCore.GetObjectProperties(SceneRootNode);
            List<SceneNodeProp> itemList = new List<SceneNodeProp>();
            RootProp = new SceneNodeProp(SceneRootNode);
            itemList.Add(RootProp);
            SceneViewTree.ItemsSource = itemList;
        }

        private void SceneViewTree_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            var item = (SceneNodeProp)SceneViewTree.SelectedItem;
            string Name = item.DisplayName;
            ActiveNode = item.SceneNode;
        }
    }

    /// <summary>
    /// Scene Node property item.
    /// </summary>
    public class SceneNodeProp
    {
        public string Icon { get; set; }
        public string DisplayName { get; set; }
        public string NodeType { get; set; }
        public IntPtr SceneNode = IntPtr.Zero;
        public List<SceneNodeProp> Children { get; set; }
        public SceneNodeProp(IntPtr node)
        {
            SceneNode = node;
            DisplayName = SakuraCore.GetStringProp(SceneNode, "Name");
            Children = new List<SceneNodeProp>();
            if (node != null)
            {
               for(uint i = 0; i < SakuraCore.GetChildrenNum(SceneNode); i++)
               {
                    IntPtr cd = SakuraCore.GetSceneNode(SceneNode, i);
                    var childNode = new SceneNodeProp(cd);
                    Children.Add(childNode);
               }
            }
        }
    }
}
