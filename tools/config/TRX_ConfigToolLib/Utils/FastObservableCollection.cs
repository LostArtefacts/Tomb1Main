using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;

namespace TRX_ConfigToolLib.Utils;

public class FastObservableCollection<T> : ObservableCollection<T>
{
    public FastObservableCollection()
        : base() { }

    public FastObservableCollection(IEnumerable<T> collection)
        : base(collection) { }

    public virtual void RemoveAll()
    {
        if (Items.Any())
        {
            Items.Clear();
        }
    }

    public virtual void ReplaceCollection(IEnumerable<T> collection)
    {
        if (collection == null || collection.SequenceEqual(Items))
        {
            return;
        }

        if (!collection.Any())
        {
            Clear();
            return;
        }

        Items.Clear();

        int oldCount = Count;
        foreach (T item in collection)
        {
            Items.Add(item);
        }

        if (Count != oldCount)
        {
            OnPropertyChanged(new PropertyChangedEventArgs(nameof(Count)));
        }
        OnPropertyChanged(new PropertyChangedEventArgs("Item[]"));
        OnCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
    }
}
