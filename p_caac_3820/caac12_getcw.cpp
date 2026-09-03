/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    1.0
Date:		2012-2-9
Description:获取财务上个月的平均价格
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac12_getcw)
//-EP_SYSTEM_HEAD_END

int f_caac12_getcw(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{

	CTracer log(__FUNCTION__);


	int doFlag = 0;
	int fetchRowCount = 0;
	CString  dept_code(""), prod_date, prod_time;

	CString  sqlstr("");
	CString  sqlstr1("");


	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq1(conn);

	try
	{

		dept_code = bcls_rec->Tables[0].Rows[0]["dept_code"].ToString();
		prod_date = CDateTime::Now().ToString("yyyyMMddHHmmss");
		//新增前先删除tcaac12表实际单价(原辅料)
		sqlstr = " delete from tcaac12 where price_terms='PJ' and mat_code in(select distinct prod_code from tsiac06 "
			" where mat_code in(select distinct mat_code from tacacmx4) and length(mat_code)<10) and dept_code='" + dept_code + "' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();
		//(坯料)
		sqlstr = " delete from tcaac12 where price_terms='PJ' and mat_code in(select distinct mat_code from tacacmx4 where length(mat_code)>10) and dept_code='" + dept_code + "' ";
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", "", "sqlstr={0}", sqlstr);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//通过当前系统时间来查询财务账期
		sqlstr = " select distinct account_period from tsifi08 "
			" where 1=1 "
			" and valid_start_date<='" + prod_date + "' "
			" and valid_end_date>='" + prod_date + "' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("prod_date", prod_date);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			prod_time = cmd_inq.GetString(1);
		}
		cmd_inq.Close();

		//通过当前财务账期来找出上一个账期
		sqlstr = " select to_char(add_months(to_date('" + prod_time + "','yyyyMM'),-1),'yyyyMM')  from  dual ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			prod_time = cmd_inq.GetString(1);
		}
		cmd_inq.Close();

		//通过财务上一个账期来查询财务收发存平均价插入到tcaac12表(原辅料)
		sqlstr = " insert into tcaac12(mat_code,mat_name,dept_code,price_unit,price_terms,valid_flag) "
			" select distinct t3.mat_code,t3.mat_name,'" + dept_code + "',round(decode(period_end_qty,0,0,period_end_amt/period_end_qty),4) price,'PJ','1' from tacacmx4 t1,tsiac06 t2,tcaac03 t3 "
			" where  t1.mat_code=t2.mat_code and t2.prod_code=t3.mat_code and length(t1.mat_code)<10 "
			" and t1.company_code=(select code_desc_2_content from tep0002 where code_class='CAS2' and code_desc_1_content='" + dept_code + "') and account_period = '"+prod_time+"'  "
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();		

		//通过财务上一个账期来查询财务收发存平均价插入到tcaac12表(坯料)
		sqlstr = " insert into tcaac12(mat_code,mat_name,dept_code,price_unit,price_terms,valid_flag) "
			" select distinct t1.mat_code,t1.mat_name,'" + dept_code + "',round(decode(period_end_qty,0,0,period_end_amt/period_end_qty),4) price,'PJ','1' from tacacmx4 t1,tcaac03 t2 "
			" where  t1.mat_code=t2.mat_code and length(t1.mat_code)>10 "
			" and company_code=(select code_desc_2_content from tep0002 where code_class='CAS2' and code_desc_1_content='" + dept_code + "') "
			" and account_period = '" + prod_time + "' ";
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", "", "sqlstr={0}", sqlstr);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应

		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	s.flag = doFlag;
	bcls_ret->SetSYS(s);
	return doFlag;
}