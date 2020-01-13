using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SakuraWPF.UserControls
{
    public interface ISPropertyInterface
    {
        SakuraCore.SPropertyData PropertyData { get; set; }

        void Initialize(SakuraCore.SPropertyData property);

        void UpdateValue();
    }
}
