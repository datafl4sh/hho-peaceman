#pragma once

#include <vector>

#include "mesh.hpp"
#include "mesh_storage.hpp"

#include "ident.hpp"
#include "point.hpp"

namespace hho {

template<size_t DIM, size_t CODIM>
class generic_element;

/* type traits for the elements */
template<typename T>
struct generic_element_traits;

template<size_t DIM, size_t CODIM>
struct generic_element_traits<generic_element<DIM, CODIM>>
{
    static_assert(CODIM < DIM, "generic_element_traits: CODIM must be less than DIM");

    typedef generic_element<DIM, CODIM+1>                           subelement_type;
    typedef identifier<generic_element<DIM,CODIM>, ident_impl_t, 0> id_type;
    static const size_t dimension = DIM;
    static const size_t codimension = CODIM;
};

template<size_t DIM>
struct generic_element_traits<generic_element<DIM, DIM>>
{
    typedef identifier<generic_element<DIM,DIM>, ident_impl_t, 0>   id_type;
    static const size_t dimension = DIM;
    static const size_t codimension = DIM;
};



/* element base class (for subelements) */
template<typename T>
class generic_element_base
{
    typedef typename generic_element_traits<T>::subelement_type::id_type    sub_id_type;
    typedef point_identifier<generic_element_traits<T>::dimension>          point_id_type;
protected:
    std::vector<sub_id_type>            m_sids_ptrs;
    std::vector<point_id_type>          m_pts_ptrs;

public:
    typedef typename std::vector<sub_id_type>::iterator         sid_iterator;
    typedef typename std::vector<sub_id_type>::const_iterator   const_sid_iterator;

    generic_element_base()
    {}

    generic_element_base(std::initializer_list<sub_id_type> l)
        : m_sids_ptrs(l)
    {}

    explicit generic_element_base(const std::vector<sub_id_type>& sids_ptrs)
        : m_sids_ptrs(sids_ptrs)
    {}

    template<typename Itor>
    void set_subelement_ids(const Itor e_begin, const Itor e_end)
    {
        m_sids_ptrs = std::vector<sub_id_type>(e_begin, e_end);
    }

    /* All this point-related stuff is simply horrible, because points should
     * be automatically derived from lower-dimensional elements. However,
     * this is not possible because points must be stored in a specific order,
     * otherwise geometry-related computations fail.
     *
     * No, in 2D they won't fail! But what about 3D?
     */
    template<typename Itor>
    void set_point_ids(const Itor p_begin, const Itor p_end)
    {
        m_pts_ptrs = std::vector<point_id_type>(p_begin, p_end);
    }

    std::vector<point_id_type>
    point_ids(void) const
    {
        assert( m_pts_ptrs.size() != 0 );
        return m_pts_ptrs;
    }

    sid_iterator        subelement_id_begin() { return m_sids_ptrs.begin(); }
    sid_iterator        subelement_id_end()   { return m_sids_ptrs.end(); }
    const_sid_iterator  subelement_id_begin() const { return m_sids_ptrs.begin(); }
    const_sid_iterator  subelement_id_end()   const { return m_sids_ptrs.end(); }

    size_t              subelement_size() const { return m_sids_ptrs.size(); }
};

/* element class, CODIM < DIM case */
template<size_t DIM, size_t CODIM>
class generic_element : public generic_element_base<generic_element<DIM, CODIM>>
{
    static_assert(CODIM <= DIM, "generic_element: CODIM > DIM does not make sense");

public:
    typedef typename generic_element_traits<generic_element>::subelement_type  subelement_type;
    typedef typename generic_element_traits<generic_element>::id_type          id_type;

    generic_element()
    {}

    generic_element(std::initializer_list<typename subelement_type::id_type> l)
        : generic_element_base<generic_element<DIM, CODIM>>(l)
    {}

    explicit generic_element(const std::vector<typename subelement_type::id_type>& sids_ptrs)
        : generic_element_base<generic_element<DIM, CODIM>>(sids_ptrs)
    {}

    bool operator<(const generic_element& other) const
    {
        return this->m_sids_ptrs < other.m_sids_ptrs;
    }
};

/* element class, CODIM == DIM case */
template<size_t DIM>
class generic_element<DIM, DIM>
{
    point_identifier<DIM>    m_assoc_point;

public:
    typedef typename generic_element_traits<generic_element>::id_type          id_type;

    generic_element()
    {}

    explicit generic_element(point_identifier<DIM> assoc_point)
    {
        m_assoc_point = assoc_point;
    }

    std::vector<point_identifier<DIM>>
    point_ids(void) const
    {
        return std::vector<point_identifier<DIM>>{{m_assoc_point}};
    }

    bool operator<(const generic_element& other) const
    {
        return this->m_assoc_point < other.m_assoc_point;
    }
};

} // namespace hho

/* Output streaming operator for elements */
template<size_t DIM, size_t CODIM>
std::ostream&
operator<<(std::ostream& os, const hho::generic_element<DIM, CODIM>& e)
{
    os << "generic_element<" << DIM << "," << CODIM << ">: ";
    for (auto itor = e.subelement_id_begin();
         itor != e.subelement_id_end();
         itor++)
        os << *itor << " ";

    return os;
}

template<size_t DIM>
std::ostream&
operator<<(std::ostream& os, const hho::generic_element<DIM, DIM>& e)
{
    os << "element<" << DIM << "," << DIM << ">: ";
    return os;
}
